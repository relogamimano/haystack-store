import re


from robot.libraries.BuiltIn import BuiltIn
from robot.libraries.OperatingSystem import OperatingSystem

class Utils:
    def __init__(self, exe):
        self.exe = exe
        self.builtin = BuiltIn()
        self.os = OperatingSystem()

    def convert_regexp(self, regex):
        return f"{re.escape(self.exe)} <disk> {regex}"

    def binary_files_should_be_equal(self, expected, actual):
        expected = self.os.get_binary_file(expected)
        actual = self.os.get_binary_file(actual)
        self.builtin.should_be_equal(expected, actual)
