## HDID, windows storage media inventory app :-)

Copyright (c) 2021 Adrian Petrila, YO3GFH<br>
    
Windows GUI app to enumerate storage media in your computer.
Built with Pelle's C compiler system.

---------
                       
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

---------

**Features**

* hdid.exe will enumerate storage media (disk drives) on your PC,
along with other (hopefully) meaningful info (size, serial no., 
interface, etc.). I've built it specifically to read serial numbers
from my HDD's. It uses the WMI (Windows Management Instrumentation).

**Note**
* hdd serial numbers, as reported by the OS, may have bytes swapped :-)
Reason? Unknown to me.. for example a ssd connected on a controller on
my MB is detected as IDE, with serial 0520B677C5505E43; if connected to
the other controller, same ssd is detected as SCSI with serial 
50026B775C05E534. Looks different, but in fact the bytes are reversed as
if one time it's little endian, then the other time it's big endian.
So HDID displays them both, for your viewing pleasure :-)) And if it
wasn't enough, sometimes thre reported SN is displayed as hex bytes...
HDID tries to display that as well, when applicable.

