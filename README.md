<p align="center">
  <img src="https://user-images.githubusercontent.com/2729145/221429873-7b2b620e-b3c9-4c87-830d-fbd1e96891d2.png" height="250" />  
  <br/><br/>

</p>
<hr/>

# Plant TV
Lighting system based on sun position for a plant inside a TV

## Description
In this project, an electronic system consisting of an ESP32 microcontroller connected to a ring of RGB LEDs has been installed in order to emulate the solar cycles inside a yellow TV. 

The microcontroller continuously computes the position of the sun in the geolocation of the TV. Understanding that it is not possible to imitate the sun's spectrum with RGB LEDs, the colorimetric parameter CCT, which defines the hue of the light, is copied.

The system can be observed below at two different times: during midday (left) and at night (right).

![comparison](https://user-images.githubusercontent.com/2729145/221429470-4f9292c1-06cd-45da-9d8b-0cf9a31f4e60.JPG)

## Material
Microcontroller ESP32: https://www.aliexpress.com/item/1005004879572949.html

Low-power LED ring (WS2812B): https://www.aliexpress.com/item/1005002328875763.html
