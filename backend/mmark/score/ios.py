import logging
log = logging.getLogger("mmark.score")

def parse_platform(platform):
    """
    Parses the iOS platform string of format "deviceFamily,version" 
    (eg. iPad3,1) and returns a tuple (model, product_name). If the
    supplied platform cannot be parsed, None is returned.

    See: http://theiphonewiki.com/wiki/Models
    """
    # 'other' devices
    if platform == "iFPGA":
        return ("iFPGA", None)

    # iPhones
    if platform == "iPhone1,2":
        return ("iPhone", "3G")
    if platform.startswith("iPhone2"):
        return ("iPhone", "3GS")
    if platform.startswith("iPhone3"):
        return ("iPhone", "4")
    if platform.startswith("iPhone4"):
        return ("iPhone", "4S")
    if platform == "iPhone5,3" or platform == "iPhone5,4":
        return ("iPhone", "5C")
    if platform.startswith("iPhone5"):
        return ("iPhone", "5")
    if platform.startswith("iPhone6"):
        return ("iPhone", "5S")
    if platform.startswith("iPhone"):
        return ("iPhone", "unknown")

    # iPods
    if platform.startswith("iPod1"):
        return ("iPod", "1G")
    if platform.startswith("iPod2"):
        return ("iPod", "2G")
    if platform.startswith("iPod3"):
        return ("iPod", "3G")
    if platform.startswith("iPod4"):
        return ("iPod", "4G")
    if platform.startswith("iPod5"):
        return ("iPod", "5G")
    if platform.startswith("iPod"):
        return ("iPod", "unknown")
    
    # iPads
    if platform.startswith("iPad1"):
        return ("iPad", "1G")
    if platform == "iPad2,5" or platform == "iPad2,6" or platform == "iPad2,7":
        return ("iPad Mini", "1G")
    if platform.startswith("iPad2"):
        return ("iPad", "2G")
    if platform == "iPad3,4" or platform == "iPad3,5" or platform == "iPad3,6":
        return ("iPad", "4G")
    if platform.startswith("iPad3"):
        return ("iPad", "3G")
    if platform == "iPad4,1" or platform == "iPad4,2":
        return ("iPad", "Air")
    if platform == "iPad4,4" or platform == "iPad4,5":
        return ("iPad Mini", "2G")

    # AppleTVs
    if platform.startswith("AppleTV2"):
        return ("Apple TV", "2G")
    if platform.startswith("AppleTV"):
        return ("Apple TV", "3G")
    
    log.debug("Could not parse platform '%s'" % platform)
    return None
