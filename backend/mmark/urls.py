from django.conf.urls import patterns, include, url
import mmark.score.urls as score
from django.conf import settings

urlpatterns = patterns('',
                       (r'', include(score)),
)
