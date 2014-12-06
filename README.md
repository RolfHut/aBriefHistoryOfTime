# A brief history of time

This is the source code for both the Spark Core and for the Arduino used in our entry to the instructables Enchanted Objects competition. In this competition, contesters are asked to add a bit of magic to an existing popular culture item. We chose to add magic to an already magical item: The TARDIS. 

In our build, users can enter a day and a month by turning two physical dials on a TARDIS console. The day/month is displayed on an LCD screen. When a button is pressed, the TARDIS will make the typical "travel through time and space" sound. After that, The Doctor will read aloud a fact from that data from a random year.

We achieve this by having a Spark Core lookup the specific date on wikipedia, parse the response and send it to a EMIC2 text to speech module. The arduino is equiped with a musicmaker shield from adafruit and plays the TARDIS sound while the Spark is connecting to Wikipedia.

See our object in action here: https://www.youtube.com/watch?v=PLx_bzKX1yk

For more detailed information, including photo's, of the build, see the instructable here: www.instructables.com/id/a-brief-history-of-time

## Open Source Licenses

Original code in this repository is licensed by Rolf Hut under the Apache License, Version 2.0.
See LICENSE for more information.

This app uses several Open Source libraries. See comments in individual files for more information.
