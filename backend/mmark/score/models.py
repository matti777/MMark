from django.db import models

"""
ALTER TABLE DeviceType ADD COLUMN platform_info VARCHAR(255) DEFAULT NULL
 AFTER platform;

ALTER TABLE MMark13_Score ADD COLUMN geo_country_code VARCHAR(8) DEFAULT NULL AFTER geo_country;
"""

class DeviceType(models.Model):
    """
    Hardware/software information for a physical device.

    Possible values for device_type field are: 
    "mobilephone", "minitablet", "tablet", "other"
    """
    added = models.DateTimeField(auto_now_add=True)
    disabled = models.BooleanField(default=False)
    platform = models.CharField(max_length=32, blank=False)
    platform_info = models.CharField(max_length=255, null=True)
    manufacturer = models.CharField(max_length=100, blank=True, null=True)
    model = models.CharField(max_length=100, blank=True, null=True)
    product_name = models.CharField(max_length=100, blank=True, null=True)
    os_version = models.CharField(max_length=100, blank=True, null=True)
    device_type = models.CharField(max_length=100, blank=False, null=False)
    total_ram = models.IntegerField(blank=True, null=True) # in kB
    num_cpu_cores = models.IntegerField(blank=True, null=True)
    cpu_type = models.CharField(max_length=100, blank=True, null=True)
    cpu_max_frequency = models.IntegerField(blank=True, null=True) # in MHz
    screen_width = models.IntegerField(blank=True, null=True) # in pixels
    screen_height = models.IntegerField(blank=True, null=True) # in pixels
    gl_vendor = models.CharField(max_length=255)
    gl_renderer = models.CharField(max_length=255)
    gl_version = models.CharField(max_length=255)
    glsl_version = models.CharField(max_length=255)
    missing_features = models.CharField(max_length=1024, null=True)

    class Meta:
        db_table = "DeviceType"

    def __unicode__(self):
        return (("DeviceType (%s): manufacturer: %s model: %s, " + 
                 "product_name: %s") %
                (self.device_type, self.manufacturer, 
                 self.model, self.product_name))

    def get_model_name(self):
        return "%s %s" % (self.model, self.product_name)

    def get_display_name(self):
        return "%s %s %s" % (self.manufacturer, self.model, self.product_name)

class Score(models.Model):
    added = models.DateTimeField(auto_now_add=True)
    uuid = models.CharField(max_length=36, unique=True)
    client_submit_id = models.CharField(max_length=36, unique=True)
    version = models.CharField(max_length=8, blank=False)
    device_type = models.ForeignKey(DeviceType)
    user_name = models.CharField(max_length=100, blank=True, null=True)
    geo_latitude = models.DecimalField(max_digits=18, decimal_places=15, 
                                       blank=True, null=True)
    geo_longitude = models.DecimalField(max_digits=18, decimal_places=15, 
                                        blank=True, null=True)
    geo_city = models.CharField(max_length=100, blank=True, null=True)
    geo_country = models.CharField(max_length=100, blank=True, null=True)
    geo_country_code = models.CharField(max_length=8, null=True)
    total_score = models.IntegerField()
    loadtime_score = models.IntegerField()
    fractal_score = models.IntegerField()
    fractal_loadtime = models.DecimalField(max_digits=6, decimal_places=3)
    fractal_num_images = models.IntegerField()
    fillrate_score = models.IntegerField()
    fillrate_loadtime = models.DecimalField(max_digits=6, decimal_places=3)
    chess_score = models.IntegerField()
    chess_loadtime = models.DecimalField(max_digits=6, decimal_places=3)
    chess_fps = models.DecimalField(max_digits=6, decimal_places=2)
    mountains_score = models.IntegerField()
    mountains_loadtime = models.DecimalField(max_digits=6, decimal_places=3)
    mountains_fps = models.DecimalField(max_digits=6, decimal_places=2)
    unlighted_fillrate = models.DecimalField(max_digits=10, decimal_places=2)
    vertex_lighted_fillrate = models.DecimalField(max_digits=10, 
                                                  decimal_places=2)
    pixel_lighted_fillrate = models.DecimalField(max_digits=10, 
                                                 decimal_places=2)
    mapped_lighted_fillrate = models.DecimalField(max_digits=10, 
                                                  decimal_places=2)

    class Meta:
        db_table = "MMark13_Score"

    def __unicode__(self):
        return ("Score: total score: %d, version: %s, client_submit_id: %s" % 
                (self.total_score, self.version, self.client_submit_id))

    def get_name_and_loc(self):
        if self.user_name:
            if self.geo_country:
                return "%s (%s)" % (self.user_name, self.geo_country)
            else:
                return self.user_name
        else:
            return None

class NewsItem(models.Model):
    added = models.DateTimeField(auto_now_add=True)
    text = models.TextField()

    class Meta:
        db_table = "NewsItem"
