Need to start the http.server on localhost to simulate URL behavior for smiley.png and in browser need to open as http://localhost:8000/canvas.html

C:\Users\HI>python
Python 3.13.9 (tags/v3.13.9:8183fa5, Oct 14 2025, 14:09:13) [MSC v.1944 64 bit (AMD64)] on win32
Type "help", "copyright", "credits" or "license" for more information.
>>> quit

C:\Users\HI>cd C:\ARTR\Assignments\ARTR01-039\02-WebGPU\07-Texture

C:\ARTR\Assignments\ARTR01-039\02-WebGPU\07-Texture>dir
 Volume in drive C has no label.
 Volume Serial Number is 50C4-F93E

 Directory of C:\ARTR\Assignments\ARTR01-039\02-WebGPU\07-Texture

08/19/2026  10:36 PM    <DIR>          .
08/19/2026  10:20 PM    <DIR>          ..
08/05/2026  11:18 PM               611 canvas.html
08/20/2026  12:14 AM            34,712 canvas.js
07/22/2026  10:31 PM            48,054 gl-matrix-min.js
07/22/2026  10:31 PM           174,379 gl-matrix.js
09/04/2025  01:53 AM             7,671 Smiley.png
               5 File(s)        265,427 bytes
               2 Dir(s)  88,641,081,344 bytes free

C:\ARTR\Assignments\ARTR01-039\02-WebGPU\07-Texture>python -m http.server 8000
Serving HTTP on :: port 8000 (http://[::]:8000/) ...

Keyboard interrupt received, exiting.

C:\ARTR\Assignments\ARTR01-039\02-WebGPU\07-Texture>python -m http.server 8000
Serving HTTP on :: port 8000 (http://[::]:8000/) ...
::1 - - [20/Aug/2026 00:23:17] "GET /canvas.html HTTP/1.1" 200 -