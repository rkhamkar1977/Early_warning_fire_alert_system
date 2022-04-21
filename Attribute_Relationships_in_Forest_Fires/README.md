## Project Overview

Forest fires are the cause of massive flora, fauna, and economical losses every year. They also contribute to global warming and endanger human lives. While more than 85% of forest fires are caused by human activities, as has been stated in numerous literatures, multiple meteorological factors contribute to its generation and persistence as well, and this project has been undertaken to quantify those relationships and utilizing those results, build an economical and sustainable early-warning fire alert system which can minimize those losses.

## Dataset

This work is utilizing the dataset given by Cortez and Morais (2007), which documents forest fire data from Montesinho natural park, Northeast Portugal. There are 517 entries in the dataset and the studied dataset consists of monthly and daily forest fire count, meteorological observations, spatial axis coordinates, and soil moisture indices.

## Exploratory Data Analysis  

The aim of our EDA is to determine:

1. Day and month in which generation of forest fires was maximum.
2. Correlation among different meteorological parameters and forest fires.
3. Which meteorological parameters contribute the most towards increasing the area burned by forest fires.


## Attribute Information
For this dataset you are provided with the following features/attributes of a park:

**X** - x-axis spatial coordinate within the Montesinho park map: 1 to 9

**Y** - y-axis spatial coordinate within the Montesinho park map: 2 to 9

**month** - month of the year: "jan" to "dec"

**day** - day of the week: "mon" to "sun"

**FFMC** - Fine Fuel Moisture Code (FFMC) index from the FWI system: 18.7 to 96.20. FFMC is a numeric rating of the moisture content of litter and other cured fine fuels. This code is an indicator of the relative ease of ignition and the flammability of fine fuel. [Canadian Forest Fire Weather Index (FWI) System]

**DMC** - Duff Moisture Code (DMC) index from the FWI system: 1.1 to 291.3. DMC is a numeric rating of the average moisture content of loosely compacted organic layers of moderate depth. This code gives an indication of fuel consumption in moderate duff layers and medium-size woody material. [Canadian Forest Fire Weather Index (FWI) System]

**DC** - Drought Code (DC) index from the FWI system: 7.9 to 860.6. DC is a numeric rating of the average moisture content of deep, compact organic layers. This code is a useful indicator of seasonal drought effects on forest fuels and the amount of smoldering in deep duff layers and large logs. [Canadian Forest Fire Weather Index (FWI) System]

**ISI** - Initial Spread Index (ISI) index from the FWI system: 0.0 to 56.10. ISI is a numeric rating of the expected rate of fire spread. It combines the effects of wind and the FFMC on rate of spread without the influence of variable quantities of fuel. [Canadian Forest Fire Weather Index (FWI) System]

**temp** - temperature in Celsius degrees: 2.2 to 33.30

**RH** - relative humidity in %: 15.0 to 100

**wind** - wind speed in km/h: 0.40 to 9.40

**rain** - outside rain in mm/m2 : 0.0 to 6.4

**area** - the burned area of the forest (in hectares): 0.00 to 1090.84

## Reference

[Cortez and Morais, 2007] P. Cortez and A. Morais. A Data Mining Approach to Predict Forest Fires using Meteorological Data. In J. Neves, M. F. Santos and J. Machado Eds., New Trends in Artificial Intelligence, Proceedings of the 13th EPIA 2007 - Portuguese Conference on Artificial Intelligence, December, Guimarães, Portugal, pp. 512-523, 2007. APPIA, ISBN-13 978-989-95618-0-9. Available at: [Web Link]

Dataset is publicly available at: <https://archive.ics.uci.edu/ml/datasets/Forest+Fires>.
