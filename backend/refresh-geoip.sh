#!/bin/sh

# Gets latest versions of the GeoIP free datafiles

rm GeoLiteCity.dat.gz
wget http://www.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
gunzip -f GeoLiteCity.dat.gz

rm GeoIP.dat.gz
wget http://www.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz
gunzip -f GeoIP.dat.gz

rm GeoIPv6.dat.gz
wget http://www.maxmind.com/download/geoip/database/GeoIPv6.dat.gz
gunzip -f GeoIPv6.dat.gz
