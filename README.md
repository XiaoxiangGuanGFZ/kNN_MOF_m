# kNN_MOF_m

## Introduction
a non-parametric model for temporal disaggregation (here daily2hourly) of meteorological records at multiste scale

## The variables

We have test the performance of this model in disaggregating the following meteorological variables in Germany:

- air temperature
- air pressure
- relative humidity
- wind speed
- solar radiation


## Algorithm

The explanation of the terms:

- kNN: k-nearest neighbor resampling
- MOF: method of fragments
- m: for meteorological variables

The disaggregation model is developed, inspired from analogy principle, where the suitable fragments from hourly observation are filtered out to redistribute daily records into hourly scale. Structural Similarity Index Measure (SSIM) is applied in the model to quantify the resemblence between candidates and the day to disaggregate. See for more details.  

## How-to-use
### software built up
The model is coded in C programming language. The model can be built up with the following instructions:

first change the work space to the model directory, and then use CMake tool to compile the source codes and built the executable. The compiler I used is GCC from MinGW. 

`cd ./kNN_MOF_m/scr/`

`cmake -G "MinGW Makefiles" .`

`mingw32-make`

### data preparation

See `./kNN_MOF_m/data_example/` for the example data. One data file contains the multisite daily records to be disaggregated and another comprises of hourly observation supplying the subdaily fragments. An extra configuration file `gp.txt` is for the model to take in the parameters which control the model behaviours. 


## Reference

## Authors

[Xiaoxiang Guan](https://www.gfz-potsdam.de/staff/guan.xiaoxiang/sec44)

E-mail: guan@gfz-potsdam.de

