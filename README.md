# haystack-store
Haystack store file system


# What I built :
● A very simplified version of the Haystack store
● Stored as one file with
○ Header
○ Index block with image names & offset, for different resolution
○ Data (JPEG images of different resolution)
● Accessible via a command-line shell
● And also via a multi-threaded web server
○ socket layer programming
○ raw HTTP layer
