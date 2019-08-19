
# YouTube communications
# see also https://developers.google.com/youtube/v3/quickstart/python

live_chat_id = 'ajsklajlsjalsjalsal'  # Live chat id
api_service_name = "youtube"
api_version = "v3"
client_secrets_file = "client_secret.json"  # this downloaded from Google
# once created interactively credential are stored to this file
credentials_file_name = 'credentials.json'

scopes = ["https://www.googleapis.com/auth/youtube.force-ssl"]


# AIS
# About the data source https://www.digitraffic.fi/meriliikenne/#/alusten-sijaintitiedot

directions = {0: "North", 45: "Northeast", 90: "East", 135: "Southeast",
              180: "South", 225: "Southwest", 270: "West", 315: "Northwest", 360: "North"}


# https://api.vtexplorer.com/docs/ref-aistypes.html
shipTypes = {
    0:  "Type not available",
    30:	"Fishing",
    31:	"Towing",
    32:	"Towing: length exceeds 200m or breadth exceeds 25m",
    33:	"Dredging or underwater ops",
    34:	"Diving ops",
    35:	"Military ops",
    36:	"Sailing",
    37:	"Pleasure Craft",
    50:	"Pilot Vessel",
    51: "Search and Rescue vessel",
    52: "Tug",
    53:	"Port Tender",
    54:	"Anti-pollution equipment",
    55:	"Law Enforcement",
    56:	"Spare - Local Vessel",
    57:	"Spare - Local Vessel",
    58:	"Medical Transport",
    59:	"Noncombatant ship according to RR Resolution No. 18",
    60:	"Passenger",
    61:	"Passenger, Hazardous category A",
    62:	"Passenger, Hazardous category B",
    63:	"Passenger, Hazardous category C",
    64:	"Passenger, Hazardous category D",
    70:	"Cargo",
    80:	"Tanker",
    90:	"Other Type"
}


min_ship_report_interval_secs = 3600
update_period_secs = 60

min_speed_to_report = 1  # do not report docked vessels

location_query_base = 'https://meri.digitraffic.fi/api/v1/locations/latitude/{:f}/longitude/{:f}/radius/{:.2f}/from/{:s}'
vessel_query_base = 'https://meri.digitraffic.fi/api/v1/metadata/vessels/{:s}'
