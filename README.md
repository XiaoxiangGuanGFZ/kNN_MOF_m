## Algorithm

for solar radiation, 
conditioning solar radiation disaggregation on air temperature doesn't bring extra value to the performance. 


there should exists a theoretical maxima of hourly radiation during one hour the surface can receive.
The received solar radiation received by the ground surface reaches maxima when the sun shines rays
directly (or perpendicularly), with no cloudness. 
So, during solar radiation disaggregation, the theoretical broundry should be considered when we select 
the candidates.

so, before SSIM, the candidates with fragments that could lead to overestimation should be ruled out. 

I suppose before disaggregation, we should make sure the hourly data itself is clean and prone to no error.
For the air pressure, the skewness estimated from hourly observation has outliers seemly. 

