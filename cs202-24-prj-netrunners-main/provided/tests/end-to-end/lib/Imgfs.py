# Author: Aurélien (2021)
# Modified by Ludovic Mermod

import os
import re
import shlex
import time

from robot.libraries.Process import Process
from robot.libraries.BuiltIn import BuiltIn
from robot.libraries.OperatingSystem import OperatingSystem
from robot.libraries.Telnet import Telnet
from Errors import Errors
from Utils import Utils

def log_crash_if_any(res):
    # Check if a crash occurred, and if so, print the stacktrace
    if res.rc == 127 and "error while loading shared libraries" in res.stdout:
        raise Exception(
            "You are missing an export LD_LIBRARY_PATH:\n{}".format(res.stdout)
        )
    elif res.rc != 0 and Imgfs.ASAN_CRASH_REGEXP.search(res.stdout):
        raise Exception("A crash occurred. Here is the output:\n{}".format(res.stdout))


class Imgfs:
    """
    Utils specific to the ckvs project
    """

    ROBOT_LIBRARY_SCOPE = "SUITE"
    ROBOT_LISTENER_API_VERSION = 2

    ASAN_CRASH_REGEXP = re.compile(r"==\d+==\s*ERROR:")
    """ Used to determine whether a crash occurred. """

    def __init__(
        self,
        exec_path: str,
        server_exec_path: str,
        enum_filename: str,
        enum_name: str,
        error_filename: str,
        error_array_name: str,
        data_dir: str,
        **kwargs,
    ):
        self.process = Process()
        self.builtin = BuiltIn()
        self.errors = Errors(
            enum_filename, enum_name, error_filename, error_array_name, **kwargs
        )
        self.os = OperatingSystem()
        self.telnet = Telnet()
        self.executable = exec_path
        self.server_executable = server_exec_path
        self.data_dir = data_dir
        self.server_process = None

        self.utils = Utils(exec_path)

        # register self as listener to log the commands that were executed
        self.ROBOT_LIBRARY_LISTENER = self
        self.logged_commands = []
        self.failures = []

        self.fuse = None

        self.test_name = None
        self.test_iteration = None

    def _start_test(self, name, attributes):
        """Initializes the library when a test case starts."""
        self.logged_commands = []
        self.failures = []

        self.test_name = name
        self.test_iteration = 0

    def _end_test(self, name, attributes):
        """Prints a message to help debugging if the test case failed."""
        if attributes["status"] == "FAIL":
            self.register_failure(None)

        i = 1
        for failure in self.failures:
            if len(failure["commands"]):
                self.builtin.log_to_console(f"\n{i})")
                self.builtin.log_to_console(
                    "*** To recreate the test case, run the following command(s):"
                )

                for cmd in failure["commands"]:
                    self.builtin.log_to_console(f"  {cmd}")
                i += 1

                if failure["message"]:
                    self.builtin.log_to_console("\n*** Error message:")
                    self.builtin.log_to_console(failure["message"])

    def register_failure(self, message):
        self.failures.append({"commands": self.logged_commands, "message": message})

    def imgfs_end_test(self):
        self.logged_commands = []
        self.test_iteration += 1

    def imgfs_dump(self):
        return (
            "dump_"
            + self.test_name.lower().replace(" ", "_")
            + (str(self.test_iteration) if self.test_iteration != 0 else "")
            + ".imgfs"
        )

    def copy_dump_file(self, name):
        dump = self.imgfs_dump()

        src = os.path.normpath(os.path.join(self.data_dir, name + ".imgfs"))
        dst = os.path.normpath(os.path.join(self.data_dir, dump))

        self.os.copy_file(src, dst)
        self.logged_commands.append(shlex.join(["cp", src, dst]))
        return dst

    def imgfs_run(
        self,
        *args,
        expected_ret=None,
        expected_string=None,
        expected_file=None,
        expected_regexp=None,
        output_file=None,
    ):
        """
        Run the u6fs executable and gives the arguments as parameters.

        :param background  Runs the executable in background. No checks on the output are done (expected_ret,
        expected_string, expected_file expected_regexp are ignored)

        :param expected_ret    The variant of the error enum expected as a return code. If None, no check is done

        :param expected_string The expected output. If None, no check is done

        :param expected_file   The file containing the expected output. If None, no check is done. The sequence VIPS in the filename will be replace by the version of VIPS.

        :param expected_regexp  A regexp that should match the output. The regex may not match the whole output, use  "^$".
        If None, no check is done
        """

        # record the command
        self.logged_commands.append(
            shlex.join([f"./{os.path.basename(self.executable)}", *args])
        )

        res = self.process.run_process(self.executable, *args, stderr="STDOUT")
        log_crash_if_any(res)

        if expected_ret:
            self.errors.compare_exit_code(res, expected_ret)

        if expected_string:
            self.builtin.should_be_equal(res.stdout.strip(), expected_string.strip())

        if expected_file:
            file = open(self.utils.translate_reference_filename(expected_file)).read()
            self.builtin.should_be_equal(res.stdout.strip(), file.strip())

        if expected_regexp:
            self.builtin.should_match_regexp(res.stdout, expected_regexp)

        if output_file:
            open(output_file, 'wb').write(res.stdout)

        return res

    def imgfs_start_server(self, file, port):
        already_running = False
        try:
            self.telnet.open_connection("localhost", port=port)
            self.telnet.close_all_connections()
            already_running = True
        except:
            pass

        if already_running:
            self.builtin.fail(f"Port {port} is already used, aborting test")

        dump = self.copy_dump_file(file)

        self.logged_commands.append(
            shlex.join([f"./{os.path.basename(self.server_executable)}", dump, port])
        )

        self.server_process = self.process.start_process(self.server_executable, dump, port)

        timeout = time.time() + 10
        while True:
            if time.time() > timeout:
                self.builtin.log(self.process.wait_for_process(self.server_process))
                self.builtin.fail("Timeout while waiting for server")

            try:
                self.telnet.open_connection("localhost", port=port)
                self.telnet.close_all_connections()
                break
            except:
                pass
        
        pass

    def imgfs_stop_server(self):
        res = self.process.terminate_process(self.server_process)
        self.builtin.log(res.rc)
        self.errors.compare_exit_code(res, "ERR_NONE")
        self.server_process = None

    def imgfs_curl(self, *args, expected_err=None, expected_file=None, output_file=None):
        self.process.process_should_be_running(self.server_process)
        
        self.logged_commands.append(shlex.join(["curl", "-i", *args]))

        curl = self.process.run_process("curl", "-i", *args, stdout="tmp.txt")
        self.builtin.should_be_equal_as_integers(curl.rc, 0)
        
        out = open("tmp.txt", "rb").read()

        if expected_err:
            expected_err_msg = "Error: " + self.errors.get_error_message(expected_err) + '\n'
            self.builtin.should_be_equal_as_strings(out, f"HTTP/1.1 500 Internal Server Error\r\nContent-Length: {len(expected_err_msg)}\r\n\r\n{expected_err_msg}".encode('utf-8'))
            
        if expected_file:
            file = open(self.utils.translate_reference_filename(expected_file), mode="rb").read()
            self.builtin.log(file.hex())
            self.builtin.log(out.hex())
            self.builtin.should_be_equal(out, file)

        if output_file:
            open(output_file, 'wb').write(out)
