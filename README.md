# Haystack store File system

## Scalable file system for big data.

This C project is the result of a semester long work for my CS202 (Computer Systems and Computer Network) cours at EPFL.

It inspired by the Facebook 2010 “Haystack” system (OSDI 2010)

Reference : https://research.facebook.com/file/1769847509889908/finding-a-needle-in-haystack-facebook-s-photo-storage.pdf

This paper describes Haystack, an object storage system optimized for Facebook’s Photos application. As of 2010, Facebook stored over 260 billion images, which translates to over 20 petabytes of data. Users uploaded one billion new photos (∼60 terabytes) each week and Facebook served over one million images per second at peak.

Haystack provides a less expensive and higher performing solution than the previous approach, which leveraged network attached storage appliances over NFS. The key observation is that this traditional design incurs an excessive number of disk operations because of metadata lookups.

It carefully reducde this per photo metadata so that Haystack storage machines can perform all metadata lookups in main memory. This choice conserves disk operations for reading actual data and thus increases overall throughput.

![image](https://github.com/user-attachments/assets/15b18af1-51c2-4f1c-9346-2eef00eeef05)
![image](https://github.com/user-attachments/assets/aefd6993-5819-4941-82ed-eca1ae800c99)
![image](https://github.com/user-attachments/assets/ded3abff-36dc-4a3b-8d8b-0dfa0f867839)

---

<b>You'll find all the code inside the cs202-24-prj-netrunners/ folder and the grade I got for this project inside the grade/ folder.</b>

## What I built :
- A very simplified version of the Haystack store
- Stored as one file with
    - Header
    - Index block with image names & offset, for different resolution
    - Data (JPEG images of different resolution)
- Accessible via a command-line shell
- And also via a multi-threaded web server
    - socket layer programming
    - raw HTTP layer

## What I learned :
- Do a guided project as a team of 2, collaborating using git
- Work with the POSIX filesystem interface
- Build a command-line interface with function pointers
- Build a simple HTTP server
- Work with libraries, e.g., libvips for image compression
- Get feedback with unit tests
- Work with debugging tools (<-- this is key)


## Requirements and packages
Install libssl-dev with the following command :

    sudo apt install libssl-dev libssl-doc


To compile it, you need to add the ssl can crypto libraries. This is done by adding the -lssl and -lcrypto flags; e.g.:

    gcc -std=c99 -o sha sha.c -lssl -lcrypto


You will need the libjson library, which allows to parse and write data in JSON format. 

If your on your own machine and haven't already done it, start by installing the libjson library:

    sudo apt install libjson-c-dev

## Web UI
To test the web server, simply launch the imgfs_server, then open http://localhost:8000/ in a web browser. You should get something like this (depending on the ImgFS with which you run your serve.






