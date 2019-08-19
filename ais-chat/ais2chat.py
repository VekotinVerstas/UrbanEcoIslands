import math
import json
from oauth2client import file, client, tools
from matplotlib import path
import requests
from datetime import datetime, date,  timedelta
import time
import urllib.parse

import settings

import os
import google_auth_oauthlib.flow
import googleapiclient.discovery
import googleapiclient.errors

from oauth2client import file, client, tools

debug_only = False  # if True only check that queries work, but do not send to youtube


# authorization
store = file.Storage(settings.credentials_file_name)


# Vasikkasaari  60.153438, 25.011680
# Kustaanmiekan suunta / vasen raja  60.143621, 24.993100
# Lonnan edusta 60.158087, 24.984143
# Katajanokasta itään 60.167353, 24.986588
boundingPoints = path.Path([(25.011680, 60.153438), (24.993100, 60.143621),
                            (24.984143, 60.158087), (24.986588, 60.167353)])


shipReports = {}

#

# Utility function for calculate radius for vessel query, https://gist.github.com/rochacbruno/2883505


def distance(origin, destination):
    lat1, lon1 = origin
    lat2, lon2 = destination
    radius = 6371  # km

    dlat = math.radians(lat2-lat1)
    dlon = math.radians(lon2-lon1)
    a = math.sin(dlat/2) * math.sin(dlat/2) + math.cos(math.radians(lat1)) \
        * math.cos(math.radians(lat2)) * math.sin(dlon/2) * math.sin(dlon/2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    d = radius * c
    return d

# Send a message via YouTube API


def send_youtube_chat_message(messageText):
    store = file.Storage(settings.credentials_file_name)
    credentials = store.get()
    if not credentials or credentials.invalid:
        flow = client.flow_from_clientsecrets(
            settings.client_secrets_file, settings.scopes)
        credentials = tools.run_flow(flow, store)

    # tsekkaa https://www.syncwithtech.org/authorizing-google-apis/

    youtube = googleapiclient.discovery.build(
        settings.api_service_name, settings.api_version, credentials=credentials)

    request = youtube.liveChatMessages().insert(
        part="snippet",
        body={
            "snippet": {
                "liveChatId": settings.live_chat_id,
                "type": "textMessageEvent",
                "textMessageDetails": {
                    "messageText": messageText
                }
            }
        }
    )
    response = request.execute()
    print(response)


def main():

    # define central point and radius for location query
    x = [p[0][0] for p in settings.boundingPoints.iter_segments()]
    y = [p[0][1] for p in boundingPoints.iter_segments()]

    centroid = (sum(x) / len(boundingPoints),
                sum(y) / len(boundingPoints))
    max_distance = 0
    for p in boundingPoints.iter_segments():
        max_distance = max(max_distance, distance(p[0], centroid))

    while True:
        # query for vessels aound the cnetroid
        utc_datetime = datetime.utcnow() - timedelta(hours=2)
        firstdate = utc_datetime.strftime("%Y-%m-%dT%H:%M:%SZ")
        request_url = settings.location_query_base.format(
            centroid[1], centroid[0], max_distance, urllib.parse.quote(firstdate))

        response = requests.get(request_url)
        locations = response.json()

        if debug_only:
            print(locations)

        for feature in locations['features']:
            mmsi = feature['mmsi']
            coordinates = feature['geometry']['coordinates']
            speed = feature['properties']['sog']

            since_last_report_secs = settings.min_ship_report_interval_secs
            if mmsi in shipReports:
                if shipReports[mmsi]:
                    since_last_report_secs = (
                        datetime.utcnow()-shipReports[mmsi]).total_seconds()

            in_view = boundingPoints.contains_points(
                [coordinates])  # is vessel inside polygon

            if ((in_view and (since_last_report_secs >= settings.min_ship_report_interval_secs) and (speed > settings.min_speed_to_report)) or debug_only):
                heading = feature['properties']['heading']
                vessel_request_url = settings.vessel_query_base.format(
                    str(mmsi))

                shipdataresp = requests.get(vessel_request_url)

                shipdata = shipdataresp.json()

                shipnName = shipdata['name']
                destination = shipdata['destination']
                direction = 0  # RFU
                shipType = shipdata['shipType']
                if shipType in settings.shipTypes:
                    shipTypeStr = settings.shipTypes[shipType]
                else:
                    shipTypeStr = 'unknown ({:d})'.format(shipType)

                heading = heading % 360  # to fix errornous headings > 360
                heading_round = int(45*((heading+23)//45))
                if heading_round in settings.directions:
                    heading_round_str = settings.directions[heading_round]
                else:
                    heading_round_str = str(heading)
                    print('Invalid rounded heading {:d} from {:d} '.format(
                        heading_round, heading))

                msgText = '{:s} ({:s}) to {:s}, speed {:3.1f} kn {:s}/{:d} '.format(shipnName, shipTypeStr,
                                                                                    destination, speed, heading_round_str, heading)
                # ° - mene läpi serialisoinnista linuxilla vaikka menee macilla
                # raise TypeError(repr(o) + " is not JSON serializable")
                # TypeError: b'VIKING XPRS (Passenger) to HELSINKI=TALLINN, speed 14.8 kn North/14\xc2\xb0 ' is not JSON serializable

                print(msgText)
                if debug_only:
                    print('debugging logic, not writing to YouTube')
                else:
                    send_youtube_chat_message(msgText)

                shipReports[mmsi] = datetime.utcnow()

        print('.')
        time.sleep(settings.update_period_secs)


if __name__ == "__main__":
    main()
