from django.http import HttpResponse
from django.utils import simplejson
import base64
from django.conf import settings

import logging
log = logging.getLogger("mmark.utils")

def expire_view_cache(view):
    """
    Expires a per-view cache entry. The argument should be a view function.
    """
    from django.core.urlresolvers import reverse
    from django.http import HttpRequest
    from django.utils.cache import get_cache_key
    from django.core.cache import cache
    
    request = HttpRequest()
    request.path = reverse(view)
    log.debug("Attempting to invalidate cache for path: %s" % request.path)
    key = get_cache_key(request)
    if key is not None:
        log.debug("Looking up cache key: %s" % key)
        if cache.get(key) is not None:
            log.debug("Expiring the entry.")
            cache.set(key, None, 0)

def check_basic_auth(request):
    auth = request.META.get("HTTP_AUTHORIZATION", None)
    log.debug("Checking auth string: %s" % auth)
    if auth:
        auth = auth.split()
        if auth[0].lower() == "basic":
            user, passwd = base64.b64decode(auth[1]).split(":")

            if user == settings.API_USERNAME:
                if passwd == settings.API_PASSWORD:
                    return None
        
    # By default return 401 Unauthorized
    response = HttpResponse()
    response.status_code = 401
    response['WWW-Authenticate'] = 'Basic realm="%s"' % settings.API_REALM
    return response

class JsonResponse(HttpResponse):
    def __init__(self, data):
        JSON_MIMETYPE="application/json; charset=utf8"
        json = simplejson.dumps(data, indent=2, ensure_ascii=False)
        super(JsonResponse, self).__init__(content=json, 
                                           mimetype=JSON_MIMETYPE)

# Context processor for providing BASE_URL in templates
def baseurl_processor(context):
    baseurl = "/"
    index = context.path.rfind('/')
    if index != -1:
        baseurl = context.path[0:index + 1]

#    log.debug("baseurl_processor(): baseurl = '%s'" % baseurl)
    return {"BASE_URL": baseurl}
