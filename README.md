Dot Bot
=======

## Description

Dot Bot is an experimental project using an **Arduino Yùn** and **Google Analytics Real Time API** (beta).  
Dot Bot is basically an adaptation of a vertical plotter that replaces the pen with a led matrix panel, and the paper with a World planisphere.

The Google Analytics Real Time API (which is still in beta) is used to know information about the current visitors of our website ([www.dotdotdot.it](http://www.dotdotdot.it)) like latitude, longitude, city and the page visited.

When none is connected, the led panel goes back to the center of the map and shows the latest tweets made by our Twitter account ([@dot_cube](https://twitter.com/dot_cube)), using the **python-twitter** library.

## Folders

* `code/` - this folder contains the "Linino-side" code, written in Python, that queries Twitter and Google Analytics Real Time
* `sketch/` - this folder contains the Arduino sketch and the libraries to support the MotorShield and the ht1632c led matrix 
* `etc/init.d/` - this folder contains the startup script that starts automatically at boot time

## Configuration

1. Edit `code/twitter_default.cfg` with your keys and secrets values obtained from [Twitter](https://dev.twitter.com/), and save it as `code/twitter.cfg`.
2. Run `code/renewtoken.py` to create a new `sample.dat` file that contains the access token to Google Analytics Real Time API.  
    There are two ways to execute the script:  
    **on your machine** - it opens the default browser and asks to login with the credentials who received access to the beta API. After this, you have to copy the generated `sample.dat` file to the Arduino Yùn, in the `code/` folder.  
    **on your Arduino Yùn** - needs the `--noauth_local_webserver` parameter. It shows a personal URL, that you have to copy and paste on your machine, and enter the verification code obtained after the login.
3. Type `/etc/init.d/ga enable` on Yùn to enable the service autostart.


## Map projection

The World planishere used in this example adopts a modified azimuthal, equal-area projection that was the seventh in a series of new projections presented by Wagner in his cartographic work, called Wagner VII.  
This was presented by Karlheinz Wagner (Germany) in 1941, the Wagner VII is a modification of the Hammer projection and is also known as the Hammer-Wagner projection.

Below a representation of the World using the Wagner VII projection.

![wagner7](http://www.giss.nasa.gov/tools/gprojector/help/projections/wagner7.gif)  
From [http://www.giss.nasa.gov/tools/gprojector/help/projections.html](http://www.giss.nasa.gov/tools/gprojector/help/projections.html)

Obviously, the movement of the led panel strictly depends on the map projection used by the planishpere, and the inversion formula should be replaced in `code/main.py` file.  
In the link below are reported formulas for all the Wagner projections inversion:  
[http://www.boehmwanderkarten.de/kartographie/is_netze_wagner_123456789_inversions.html](http://www.boehmwanderkarten.de/kartographie/is_netze_wagner_123456789_inversions.html)

## References

* [Arduino Yùn](http://arduino.cc/en/Main/ArduinoBoardYun?from=Main.ArduinoYUN)
* [Google Analytics Real Time API](https://developers.google.com/analytics/devguides/reporting/realtime/v3/)
* [python-twitter](https://github.com/bear/python-twitter)
* [Wagner VII projection](http://www.mapthematics.com/ProjectionsList.php?Projection=188)

