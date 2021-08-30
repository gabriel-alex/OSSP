import os
from dotenv import load_dotenv

load_dotenv()
print('-D AP_SSID={} AP_PASS={}'.format(os.getenv('AP_SSID', ''), os.getenv('AP_PASS', '')) )