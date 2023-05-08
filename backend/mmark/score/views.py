from django.http import HttpResponse, HttpResponseBadRequest
from django.http import HttpResponseRedirect
from django.http import HttpResponseNotFound
from django.views.decorators.http import require_GET, require_POST
from mmark.utils import JsonResponse, check_basic_auth, expire_view_cache
from django.views.decorators.csrf import csrf_exempt
from django.utils import simplejson
from django.shortcuts import render_to_response, redirect
from django.template import RequestContext
from django.core.urlresolvers import reverse
from mmark.score.models import DeviceType, Score, NewsItem
from django.conf import settings
from django.db import IntegrityError
from django.db import connection
from decimal import Decimal
from datetime import datetime
from dateutil.relativedelta import relativedelta
from mmark.score.ios import parse_platform
from django.utils import timezone 
from django.views.decorators.cache import cache_page
import uuid
import md5
import base64

import logging
log = logging.getLogger("mmark.score")

@require_GET
def overview(request):
    log.debug("overview()")

    #TODO remove!
    expire_view_cache(view_score)

    # Get latest news items
    items = NewsItem.objects.all().order_by("-added")[:5]

    return render_to_response("overview.html", 
                              {"mainlink_target": "/view_score",
                               "news_items": items}, 
                              context_instance=RequestContext(request))

def create_highscore_data(scores):
    highest = float("-inf")

    for score in scores:
        score_f = float(score["score"])
        if score_f > highest:
            highest = score_f

    # str() cast that returns "" instead of string 'None' for None
    xstr = lambda s: str(s) if s is not None else ""
    
    l = []
    for score in scores:
        score_f = float(score["score"])
        barlen = settings.GRAPH_WIDTH * (score_f / highest)
        device_disp_name = "%s %s %s" % (score["manufacturer"], score["model"],
                                         xstr(score["product_name"]))
        l.append({"uuid": score["uuid"], 
                  "name": score["user_name"], 
                  "country": score["geo_country"],
                  "country_code": score["geo_country_code"],
                  "device": device_disp_name.strip(),
                  "score": score["score"],
                  "bar_length": barlen})

    return l

def dictfetchall(cursor):
    """
    Returns all rows from a cursor as a dict
    """
    desc = cursor.description
    return [
        dict(zip([col[0] for col in desc], row))
        for row in cursor.fetchall()
    ]

def filter_devices(scores): 
    hashkey = lambda s: "%s%s%s" % (s["manufacturer"], 
                                    s["model"], s["product_name"])
    lookup = {}
    filtered_list = []

    for score in scores:
        # Check whether a matching device already exists in the lookup cache
        key = hashkey(score)
        entry = lookup.get(key)
        if entry is not None:
            if (entry["manufacturer"] == score["manufacturer"] and 
                entry["model"] == score["model"] and 
                entry["product_name"] == score["product_name"]): 
                continue
        
        # Put the new score / device in the lookup and to the end of the list
        lookup[key] = score
        filtered_list.append(score)
        if len(filtered_list) >= 10:
            return filtered_list
                                   
    return filtered_list

def get_most_recent_score():
    cursor = connection.cursor()
    cursor.execute("SELECT DT.manufacturer, DT.model, DT.product_name, " + 
                   "S.total_score AS score, S.uuid, " + 
                   "S.user_name, S.geo_country, S.geo_country_code, S.added " + 
                   "FROM MMark13_Score S, DeviceType DT WHERE " + 
                   "DT.disabled = false AND " + 
                   "S.device_type_id = DT.id ORDER BY S.added DESC LIMIT 10")
    return create_highscore_data(dictfetchall(cursor))

def get_cpu_high_score():
    cursor = connection.cursor()
    cursor.execute("SELECT DT.manufacturer, DT.model, DT.product_name, " + 
                   "S.fractal_score AS score, S.uuid, " + 
                   "S.user_name, S.geo_country, S.geo_country_code, S.added " + 
                   "FROM MMark13_Score S, DeviceType DT WHERE " + 
                   "(DT.disabled = false) AND (S.device_type_id = DT.id) AND " +
                   "(S.total_score = " + 
                   "  (SELECT MAX(S2.total_score) FROM MMark13_Score S2 " + 
                   "  WHERE S.device_type_id = S2.device_type_id)) " + 
                   "ORDER BY score DESC")

    return create_highscore_data(filter_devices(dictfetchall(cursor)))

def get_device_high_score():
    cursor = connection.cursor()
    cursor.execute("SELECT DT.manufacturer, DT.model, DT.product_name, " + 
                   "S.total_score AS score, S.uuid, " + 
                   "S.user_name, S.geo_country, S.geo_country_code, S.added " + 
                   "FROM MMark13_Score S, DeviceType DT WHERE " + 
                   "(DT.disabled = false) AND (S.device_type_id = DT.id) AND " +
                   "(S.total_score = " + 
                   "  (SELECT MAX(S2.total_score) FROM MMark13_Score S2 " + 
                   "  WHERE S.device_type_id = S2.device_type_id)) " + 
                   "ORDER BY score DESC")

    return create_highscore_data(filter_devices(dictfetchall(cursor)))

def get_phone_high_score():
    cursor = connection.cursor()
    cursor.execute("SELECT DT.manufacturer, DT.model, DT.product_name, " + 
                   "S.total_score AS score, S.uuid, " + 
                   "S.user_name, S.geo_country, S.geo_country_code, S.added " + 
                   "FROM MMark13_Score S, DeviceType DT WHERE " + 
                   "(DT.device_type = 'mobilephone') AND " + 
                   "(DT.disabled = false) AND (S.device_type_id = DT.id) AND " +
                   "(S.total_score = " + 
                   "  (SELECT MAX(S2.total_score) FROM MMark13_Score S2 " + 
                   "  WHERE S.device_type_id = S2.device_type_id)) " + 
                   "ORDER BY score DESC")
    return create_highscore_data(filter_devices(dictfetchall(cursor)))

def get_tablet_high_score():
    cursor = connection.cursor()
    cursor.execute("SELECT DT.manufacturer, DT.model, DT.product_name, " + 
                   "S.total_score AS score, S.uuid, " + 
                   "S.user_name, S.geo_country, S.geo_country_code, S.added " + 
                   "FROM MMark13_Score S, DeviceType DT WHERE " + 
                   "((DT.device_type = 'tablet') OR " + 
                   "(DT.device_type = 'minitablet')) AND " + 
                   "(DT.disabled = false) AND (S.device_type_id = DT.id) AND " +
                   "(S.total_score = " + 
                   "  (SELECT MAX(S2.total_score) FROM MMark13_Score S2 " + 
                   "  WHERE S.device_type_id = S2.device_type_id)) " + 
                   "ORDER BY score DESC")
    return create_highscore_data(filter_devices(dictfetchall(cursor)))

def get_geo_distribution():
    """
    Finds the number of score submissions per country.
    """ 
    cursor = connection.cursor()
    cursor.execute("SELECT COUNT(id) AS count, geo_country, geo_country_code "+ 
                   "FROM MMark13_Score WHERE geo_country IS NOT NULL " + 
                   "GROUP BY geo_country, geo_country_code")
    return dictfetchall(cursor)

def count_platform_scores(platform, since = None):
    q = Score.objects.filter(device_type__platform=platform)
    if since is not None:
        q = q.filter(added__gte=since)
    return q.count()    

@cache_page(60 * 60)
def view_score(request):
    uuid = request.REQUEST.get("uuid", None)

    if uuid is None:
        log.debug("Generating the complete scores page")
        num_android_scores = count_platform_scores("android")
        num_ios_scores = count_platform_scores("ios")
        month_ago = timezone.now() + relativedelta(months = -1)
        num_android_scores_month = count_platform_scores("android", month_ago)
        num_ios_scores_month = count_platform_scores("ios", month_ago)

        # Display the Scores page
        return render_to_response("scores.html", 
                                  {"mainlink_target": "/",
                                   "num_android_scores": num_android_scores,
                                   "num_ios_scores": num_ios_scores,
                                   "num_android_scores_month": 
                                   num_android_scores_month,
                                   "num_ios_scores_month": num_ios_scores_month,
                                   "countries": get_geo_distribution(),
                                   "most_recent_score": get_most_recent_score(),
                                   "cpu_score": get_cpu_high_score(), 
                                   "device_high_score": 
                                   get_device_high_score(),
                                   "phone_high_score": 
                                   get_phone_high_score(),
                                   "tablet_high_score": 
                                   get_tablet_high_score()},                    
                                  context_instance=RequestContext(request))
    else:
        # Display a single score
        log.debug("Generating a single score page for uuid: %s" % uuid)
        try:
            score = Score.objects.get(uuid=uuid)
        except Score.DoesNotExist:
            return HttpResponseNotFound()

        # If nonce present, check it
        nonce = request.REQUEST.get("nonce", None)
        if nonce is not None:
            log.debug("nonce = %s" % nonce)
            if nonce != score.client_submit_id:
                return HttpResponseBadRequest("Invalid nonce")
            if score.user_name is not None:
                if score.user_name != "":
                    # Name already set; invalidate nonce
                    nonce = None

        if request.method == "GET":
            log.debug("Viewing %s" % score)
            nameb64 = request.COOKIES.get("user_name", None)
            name = base64.b64decode(nameb64) if nameb64 is not None else None
            missing_features = score.device_type.missing_features
            if missing_features == "":
                missing_features = None
            gl_finish_err = False
            if missing_features is not None:
                gl_finish_err = "glFinish" in missing_features
    
            return render_to_response("view_score.html", 
                                      {"mainlink_target": "view_score",
                                       "missing_features": missing_features,
                                       "gl_finish_err": gl_finish_err,
                                       "name": name,
                                       "score": score, "nonce": nonce,
                                       "most_recent_score": 
                                       get_most_recent_score()}, 
                                      context_instance=RequestContext(request))
        elif request.method == "POST":
            name = request.POST.get("name", None)
            log.debug("Setting name: '%s'" % name)
            response = HttpResponseRedirect(reverse(view_score) + 
                                            "?uuid=%s" % uuid)
            if name is not None:
                log.debug("Encoding name '%s' to base64 for cookie" % name)
                nameb64 = base64.b64encode(name.encode("utf8", "ignore"))
                response.set_cookie("user_name", nameb64, 365*24*60*60*10)
                score.user_name = name
                score.save()
                
            return response
        else:
            return HttpResponseBadRequest("Invalid method")

def get_device_type(json, match_product_name):
    # int() cast that returns None for empty input
    nint = lambda s: int(s) if (s is not None and len(str(s)) > 0) else None

    platform = json["platform"]
    platform_info = json.get("platform_info", None)
    manufacturer = json["manufacturer"]
    model = json["model"]
    product_name = json["product_name"]
    os_version = json["os_version"]
    device_type = json["device_type"]
    total_ram = nint(json["total_ram"])
    num_cpu_cores = nint(json["num_cpu_cores"])
    cpu_type = json["cpu_type"]
    cpu_max_frequency = nint(json["cpu_max_frequency"])
    screen_width = nint(json["screen_width"])
    screen_height = nint(json["screen_height"])
    gl_vendor = json["gl_vendor"]
    gl_renderer = json["gl_renderer"]
    gl_version = json["gl_version"]
    glsl_version = json["glsl_version"]
    missing_features = json.get("missing_features", None)
    if missing_features is not None:
        missing_features = missing_features.strip()

    # Look for matching DeviceType
    q = DeviceType.objects.filter(manufacturer=manufacturer)
    q = q.filter(platform=platform)
    if platform_info is not None:
        q = q.filter(platform_info=platform_info)
    q = q.filter(model=model)
    if match_product_name:
        q = q.filter(product_name=product_name)
    q = q.filter(device_type=device_type)
    q = q.filter(os_version=os_version)
    q = q.filter(total_ram=total_ram)
    q = q.filter(num_cpu_cores=num_cpu_cores)
    q = q.filter(cpu_type=cpu_type)
    q = q.filter(cpu_max_frequency=cpu_max_frequency)
    q = q.filter(screen_width=screen_width)
    q = q.filter(screen_height=screen_height)
    q = q.filter(gl_vendor=gl_vendor)
    q = q.filter(gl_renderer=gl_renderer)
    q = q.filter(gl_version=gl_version)
    q = q.filter(glsl_version=glsl_version)
    q = q.filter(missing_features=missing_features)

    if len(q) == 0:
        # Add new entry and return it
        device = DeviceType(manufacturer=manufacturer, 
                            platform=platform,
                            platform_info=platform_info,
                            model=model, 
                            product_name=product_name,
                            device_type=device_type,
                            os_version=os_version,
                            total_ram=total_ram,
                            num_cpu_cores=num_cpu_cores,
                            cpu_type=cpu_type,
                            cpu_max_frequency=cpu_max_frequency,
                            screen_width=screen_width,
                            screen_height=screen_height,
                            gl_vendor=gl_vendor,
                            gl_renderer=gl_renderer,
                            gl_version=gl_version,
                            glsl_version=glsl_version,
                            missing_features=missing_features)
        device.save()
        log.debug("get_device_type(): created new entry: %s" % device)
        return device
    elif len(q) > 1:
        # Curious, there shouldnt be multiple copies
        log.error("Several identical copies of DeviceType found")

    # Return the first (and hopefully only) match
    log.debug("get_device_type(): found match: %s" % str(q[0]))
    return q[0]

def get_geo_info(client_addr):
    try:
        from django.contrib.gis.geoip import GeoIP
    except ImportError, msg:
        log.debug("Cannot import GeoIP: %s" % msg)
        return {}

    g = GeoIP()
    geo = g.city(client_addr)
    if not geo:
        geo = {}

    log.debug("GeoIP info: %s" % unicode(geo))

    return geo

def insert_score(json, client_addr):
    # Geolocate the client
    #log.debug("client_addr = %s" % client_addr);
    geo = get_geo_info(client_addr)

    # extract the 'score' node
    s = json["score"]

    # Get a DeviceType
    device_info = json["device_info"]
    match_product_name = True
    if s["version"] == "1.0" and device_info["platform"] == "ios":
        match_product_name = False
    device_type = get_device_type(device_info, match_product_name)

    # Insert a new Score entry
    score_uuid = str(uuid.uuid4())
    user_json = json["user"]

    # str() cast that returns proper None instead of string 'None'
    xstr = lambda s: str(s) if s else None

    # The str() conversions are required in python <= 2.7 for float->Decimal
    score = Score(uuid=score_uuid, 
                  client_submit_id=json["submit_id"],
                  version=s["version"],
                  device_type=device_type, 
                  user_name=user_json["name"],
                  total_score=s["total_score"], 
                  loadtime_score=s["loadtime_score"],
                  geo_latitude=xstr(geo.get("latitude", None)),
                  geo_longitude=xstr(geo.get("longitude", None)),
                  geo_city=geo.get("city", None), 
                  geo_country=geo.get("country_name", None),
                  geo_country_code=geo.get("country_code", None),
                  unlighted_fillrate=str(s["unlighted_fillrate"]),
                  vertex_lighted_fillrate=str(s["vertex_lighted_fillrate"]),
                  pixel_lighted_fillrate=str(s["pixel_lighted_fillrate"]),
                  mapped_lighted_fillrate=str(s["mapped_lighted_fillrate"]),
                  fractal_score=s["fractal_score"], 
                  fractal_loadtime=str(s["fractal_loadtime"]),
                  fractal_num_images=s["fractal_num_images"], 
                  fillrate_score=s["fillrate_score"],
                  fillrate_loadtime=str(s["fillrate_loadtime"]),
                  chess_score=s["chess_score"], 
                  chess_loadtime=str(s["chess_loadtime"]),
                  chess_fps=str(s["chess_fps"]),
                  mountains_score=s["mountains_score"], 
                  mountains_loadtime=str(s["mountains_loadtime"]),
                  mountains_fps=str(s["mountains_fps"]))

    score.save()
    log.debug("New score inserted")
    return score

def fix_json(json):
    device = json["device_info"]
    score = json["score"]

    if score["version"] == "1.0" and device["manufacturer"] == "Apple":
        # Fix the iOS client 1.0 version bug that reports wrong platform
        device["platform"] = "ios"
        
    platform_info = device.get("platform_info", None)
    if platform_info is not None and device["platform"] == "ios":
        # Find device model/product name by the platform info string
        parsed = parse_platform(platform_info)
        if parsed is not None:
            model, product_name = parsed
            log.debug("Parsed platform_info '%s' into " + 
                      "model: %s, product_name: %s" % (model, product_name))
            device["model"] = model
            device["product_name"] = product_name

@require_POST
@csrf_exempt
def upload_score(request):
    log.debug("upload_score()")

    # Verify JSON API basic auth
    res = check_basic_auth(request)
    if res:
        log.error("upload_score(): missing/bad authentication")
        return res

    # Check that the json posted matches the signature supplied
    signature = request.META.get("HTTP_X_MMARK_SIGNATURE", None)
    if signature is None:
        log.error("upload_score(): Missing signature!")
        return HttpResponseBadRequest()
    json_string = request.raw_post_data
    md5_input = json_string + settings.JSON_SALT
    my_sig = md5.new(md5_input).hexdigest()
    if my_sig != signature:
        log.error("upload_score(): Signature mismatch!")
        return HttpResponseBadRequest()

    # Parse the client's submitted data
    json = simplejson.loads(request.raw_post_data)

    # 'Fix' the submission JSON based on the protocol version to fix
    # some client bugs
    fix_json(json)

    # Get the client's (IP) address. Use HTTP_X_FORWARDED_FOR if specified
    client_addr = request.META.get("HTTP_X_FORWARDED_FOR", None)
    if not client_addr:
        client_addr = request.META["REMOTE_ADDR"]

    # Insert the score/device into the database
    score = None
    try:
        score = insert_score(json, client_addr)
    except KeyError, msg:
        log.debug("upload_score(): error while reading JSON: %s" % msg)
        return HttpResponseBadRequest("Missing item: %s" % msg)
    except IntegrityError, msg:
        # client_submit_id clash - resubmission of score. return 409 Conflict
        log.debug("upload_score(): score insertion failed: %s" % msg)
        return HttpResponse("client_submit_id resubmission", status=409)

    # Must invalidate the view_score cache now that there is new data
    log.debug("Invalidating the cached view_score page")
    expire_view_cache(view_score)

    return JsonResponse({"score_uuid": score.uuid, 
                         "nonce": score.client_submit_id});
    

