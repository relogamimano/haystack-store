# Author: Aurélien (2021)
# Modified by Ludovic Mermod

import parse as ps
from robot.libraries.Process import Process


class Errors:
    def __init__(
        self,
        enum_filename: str,
        enum_name: str,
        error_filename: str,
        error_array_name: str,
        **kwargs,
    ):
        """
        Initializes the library to be able to compare error code's enum to their integer value.
        :param: enum_filename, str, the filename of the header containing the error code enum
        :param: enum_name, str, the name of the error code enum
        :param: error_filename, str, the filename of the source file defining the error message strings
        :param: error_array_name, str, the name of the error message array
        :param: error_prefix, str, the prefix that should appear on the last line of the stdout before the error message
        """

        self.enum_mapping = None
        self.error_messages = None
        self.error_prefix = kwargs.get("error_prefix", kwargs.get("prefix", ""))

        self.__init_enum_values(enum_filename, enum_name)
        self.__init_error_messages(error_filename, error_array_name)

    def __get_content_without_comments(self, filename):
        with open(filename, mode="r", encoding="utf-8") as file:
            lines = []
            for line in file:
                for word in line.strip().split():
                    if word.startswith("//"):
                        break
                    lines.append(word)
                else:
                    continue

            content = " ".join(lines)

            while True:
                start = content.find("/*")
                if start == -1:
                    break
                end = content.find("*/")

                content = content[:start] + content[end + 2 :]

            content = content.strip()
            return content

    def __parse_enum(self, content: str) -> dict:
        values = []
        for s in content.split(","):
            s = s.strip()
            if len(s) != 0:
                values.append(s)

        res = dict()
        counter = 0
        for s in values:
            if "=" in s:
                p = ps.parse("{:w} = {:d}", s)
                counter = int(p[1])
                res[p[0]] = counter
            else:
                res[s] = counter
            counter += 1

        return res

    def __parse_messages(self, content: str) -> list:
        messages = []

        for s in content.split(","):
            start = s.index('"') + 1
            end = s.index('"', start)

            s = s[start:end]
            messages.append(s)

        return messages

    def __init_enum_values(self, filename: str, enum_name: str):
        """
        Searches the file with the given filename and parses the enum with the given enum name.
        :param: filename, str
        :param: enum_name, str
        """

        content = self.__get_content_without_comments(filename)
        self.enum_mapping = None

        enums = ps.findall("enum {} {{ {} }};", content)
        for e in enums:
            if e[0] == enum_name:
                self.enum_mapping = self.__parse_enum(e[1])

        enums = ps.findall("typedef enum {{ {} }} {};", content)
        for e in enums:
            if e[1] == enum_name:
                self.enum_mapping = self.__parse_enum(e[0])

        enums = ps.findall("typedef enum {} {{ {} }} {};", content)
        for e in enums:
            if e[0] == enum_name or e[2] == enum_name:
                self.enum_mapping = self.__parse_enum(e[1])

        if self.enum_mapping is None:
            raise Exception(
                f"Enum '{enum_name}' not found in '{filename}'.\n"
                + "Make sure your enum is defined as either:\n - enum <enum_name> { ... };\n - typedef enum { ... } <enum_name>;"
            )

    def __init_error_messages(self, filename: str, array_name: str):
        """
        Searches the file with the given filename and parses the error messages array with the given array_name
        :param: filename, str
        :param: array_name, str
        """

        content = self.__get_content_without_comments(filename)
        self.error_messages = None

        matches = ps.findall("{:w}[] = {{ {} }};", content)
        for e in matches:
            if e[0] == array_name:
                self.error_messages = self.__parse_messages(e[1])

        if self.error_messages is None:
            raise Exception(
                f"Array '{array_name}' not found in '{filename}'.\n"
                + "Make sure your array has the correct format:\n [const] <type> <array_name>[] = { ... };"
            )

    def get_enum_ordinal(self, enum_name: str):
        if self.enum_mapping is None:
            raise Exception("Enum mapping is not initialized.")
        if self.error_messages is None:
            raise Exception("Error message array is not initialized.")
        if not enum_name in self.enum_mapping:
            raise Exception(f"Enum name '{enum_name}' is not in the enum mapping.")

        return self.enum_mapping[enum_name]

    def match_signedness(self, n, pat):
        """
        Converts a number to an signed/unsigned byte, depending on pat signedness (if it is possible to determine).
        This is useful for return code comparison, as their signedness is OS-dependent
        (https://stackoverflow.com/questions/47265667/return-code-on-failure-positive-or-negative)
        """
        if pat < 0:
            return (n + 256) % 256 - 256
        elif pat >= 128:
            return (n + 256) % 256
        else:
            return n

    # ========================================================================================================
    # Public methods

    def compare_exit_code(self, res, enum_name: str):
        """
        Compares the given error code with the ordinal of the enum value
        :param: res, result object returned by the program
        :param: enum_name, str (eg. ERR_NONE)
        """
        enum_ordinal = (
            0 if enum_name == "ERR_NONE" else self.get_enum_ordinal(enum_name)
        )
        rc = self.match_signedness(res.rc, enum_ordinal)

        if rc != enum_ordinal:
            error_name = [
                name for name, ord in self.enum_mapping.items() if ord == rc
            ] or "Unknown error"
            raise Exception(
                f"Returned code {rc} ('{error_name}') != expected {enum_ordinal} ('{enum_name}'). Program output:\n{res.stdout}"
            )

    def get_error_message(self, enum_name):
        """
        Returns the message corresponding to the given enum value
        :param: enum_name, str (eg. ERR_IO)
        :return: str, the human readable message corresponding to that enum value
        """
        return self.error_messages[self.get_enum_ordinal(enum_name) - self.get_enum_ordinal("ERR_FIRST")]

    def check_error_code_and_message(self, result, enum_name):
        """
        Checks that the result object (as returned by Run Process) has the correct return code
        and a stdout that contains the corresponding error message.
        :param: result, object as returned by the Run Process keyword
        :param: enum_name, str
        """
        error_message = self.get_error_message(enum_name)
        expected = f"{self.error_prefix} {error_message}".strip()

        # assert that the result code is correct
        self.compare_exit_code(result, enum_name)

        # assert that stdout contains the correct error code
        lines = result.stdout.splitlines()
        assert (
            len(lines) > 0
        ), f"Expected stdout to contain '{expected}' but it was empty."

        # search for the last non empty line (eg. if program printed too many '\n')
        last = len(lines) - 1
        while last > 0:
            if len(lines[last].strip()) != 0:
                break
            last -= 1

        last_line = lines[last]
        assert (
            expected in last_line
        ), f"Expected last line of stdout to contain '{expected}' but was '{last}'"
