from django.conf.urls.defaults import *
from . import views

urlpatterns = patterns('',
                       (r'^$', views.overview),
                       (r'^upload$', views.upload_score),
                       (r'^view_score$', views.view_score),
)
