import re


from robot.libraries.BuiltIn import BuiltIn
from robot.libraries.Process import Process
from robot.libraries.OperatingSystem import OperatingSystem

class Utils:
    def __init__(self, exe):
        self.exe = exe
        self.process = Process()
        self.builtin = BuiltIn()
        self.os = OperatingSystem()

        self._get_vips_version()


    def _get_vips_version(self):
        res = self.process.run_process("vips", "-v")
        if res.rc == 0:
            self.vips_version_string = re.search(r"vips-(\d+\.\d+\.\d+)", res.stdout).group(1)

    def translate_reference_filename(self, filename):
        return filename.replace("VIPS", self.vips_version_string)

    def convert_regexp(self, regex):
        return f"{re.escape(self.exe)} <disk> {regex}"

    def binary_files_should_be_equal(self, expected, actual):
        expected = self.os.get_binary_file(self.translate_reference_filename(expected))
        actual = self.os.get_binary_file(actual)
        self.builtin.should_be_equal(expected, actual)
