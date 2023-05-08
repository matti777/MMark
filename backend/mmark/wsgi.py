"""
WSGI config for mmark project.

This module contains the WSGI application used by Django's development server
and any production WSGI deployments. It should expose a module-level variable
named ``application``. Django's ``runserver`` and ``runfcgi`` commands discover
this application via the ``WSGI_APPLICATION`` setting.

Usually you will have the standard Django WSGI application here, but it also
might make sense to replace the whole Django WSGI application with a custom one
that later delegates to the Django one. For example, you could introduce WSGI
middleware here, or combine a Django application with an application of another
framework.

"""
import os
import sys

# Add backend base directory to the system path
paths = os.path.abspath(os.path.split(__file__)[0]).split("/")
BACKEND_PATH = "/".join(paths[0:len(paths)-1])
print BACKEND_PATH

# Add it to system path
sys.path.append(BACKEND_PATH)    

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "mmark.settings")

# This application object is used by any WSGI server configured to use this
# file. This includes Django's development server, if the WSGI_APPLICATION
# setting points here.
from django.core.wsgi import get_wsgi_application
application = get_wsgi_application()

# Apply WSGI middleware here.
# from helloworld.wsgi import HelloWorldApplication
# application = HelloWorldApplication(application)

# Log exceptions to the console
import sys
import traceback

from django.core.signals import got_request_exception

def exception_printer(sender, **kwargs):
    print >> sys.stderr, ''.join(traceback.format_exception(*sys.exc_info()))

got_request_exception.connect(exception_printer)
