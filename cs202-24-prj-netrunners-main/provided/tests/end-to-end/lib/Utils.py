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

    def binary_files_should_be_equal(self, expected, actual):
        expected = self.os.get_binary_file(self.translate_reference_filename(expected))

    def convert_regexp(self, regex):
        return f"{re.escape(self.exe)} <disk> {regex}"

    def resize_jpeg(self, origin, output, width, height):
        res = self.process.run_process("vips", "thumbnail", origin, output, str(width), "-h", str(height))
        self.builtin.should_be_equal_as_integers(res.rc, 0)

    def resize_jpeg_http(self, origin, output, width, height):
        print(["vips", "thumbnail", origin, f"{output}.jpg", str(width), "-h", str(height)])
        res = self.process.run_process("vips", "thumbnail", origin, f"{output}.jpg", str(width), "-h", str(height))
        self.builtin.should_be_equal_as_integers(res.rc, 0)

        image = self.os.get_binary_file(f"{output}.jpg")
        file = open(output, "wb")
        file.write('HTTP/1.1 200 OK\n'.encode('utf-8'))
        file.write('Content-Type: image/jpeg\n'.encode('utf-8'))
        file.write(f'Content-Length {len(image)}\n'.encode('utf-8'))
        file.write(image)
        file.close()

    def jpeg_files_should_be_equal(self, expected, actual):
        expected = self.os.get_binary_file(expected)
        actual = self.os.get_binary_file(actual)
        sos_index = expected.find(b'\xff\xda')
        self.builtin.should_be_equal(expected[:sos_index], actual[:sos_index])
