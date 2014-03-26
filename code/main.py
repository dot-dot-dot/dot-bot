
import sys

import argparse
import httplib2
import os
import math
import time
import socket
import twitter
import ConfigParser
  
from apiclient import discovery
from oauth2client import file
from oauth2client import client
from oauth2client import tools
from urllib2 import HTTPError as HttpError
from httplib import BadStatusLine

sys.path.insert(0, '/usr/lib/python2.7/bridge/')
import json
from bridgeclient import BridgeClient as bridgeclient

parser = argparse.ArgumentParser(
    description=__doc__,
    formatter_class=argparse.RawDescriptionHelpFormatter,
    parents=[tools.argparser])


# CLIENT_SECRETS is name of a file containing the OAuth 2.0 information for this
# application, including client_id and client_secret. You can see the Client ID
# and Client secret on the APIs page in the Cloud Console:
# <https://cloud.google.com/console#/project/450330591606/apiui>
CLIENT_SECRETS = os.path.join(os.path.dirname(__file__), 'client_secrets.json')

# Set up a Flow object to be used for authentication.
# Add one or more of the following scopes. PLEASE ONLY ADD THE SCOPES YOU
# NEED. For more information on using scopes please see
# <https://developers.google.com/+/best-practices>.
FLOW = client.flow_from_clientsecrets(CLIENT_SECRETS,
  scope=[
      'https://www.googleapis.com/auth/analytics',
      'https://www.googleapis.com/auth/analytics.readonly',
    ],
    message=tools.message_if_missing(CLIENT_SECRETS))


def main(argv):
  # Parse the command-line flags.
  flags = parser.parse_args(argv[1:])

  # If the credentials don't exist or are invalid run through the native client
  # flow. The Storage object will ensure that if successful the good
  # credentials will get written back to the file.
  storage = file.Storage('/home/code/sample.dat')
  credentials = storage.get()
  if credentials is None or credentials.invalid:
    credentials = tools.run_flow(FLOW, storage, flags)

  # Create an httplib2.Http object to handle our HTTP requests and authorize it
  # with our good Credentials.
  http = httplib2.Http()
  http = credentials.authorize(http)

  # Construct the service object for the interacting with the Google Analytics API.
  service = discovery.build('analytics', 'v3', http=http)

  #Read the Config File to get the twitter keys and tokens
  config = ConfigParser.RawConfigParser()
  config.read('twitter.cfg')

  # Create a twitter.Api object that handles requests of latest tweets to show
  # when the bot is in standby
  api = twitter.Api(consumer_key=config.get('DEFAULT', 'consumer_key'), consumer_secret=config.get('DEFAULT', 'consumer_secret'),
                    access_token_key=config.get('DEFAULT', 'access_token_key'), access_token_secret=config.get('DEFAULT', 'access_token_secret'))

# 1. Create and Execute a Real Time Report
# An application can request real-time data by calling the get method on the Analytics service object.
# The method requires an ids parameter which specifies from which view (profile) to retrieve data.
# For example, the following code requests real-time data for view (profile) ID 2809964.

  value = bridgeclient()
  reload(json)
  start = 0
   
  while True:
    try:
    # Get information about current visitors
    # (eg. latitude, longitude, country, city and pageTitle)
      results = service.data().realtime().get(
        ids='ga:2809964',
        metrics='ga:activeVisitors',
        dimensions='ga:latitude,ga:longitude,ga:country,ga:city,ga:pageTitle',
        max_results=1).execute()

    # Check if user present otherwise
    # go standby and show tweets

      if results.get('rows', []):
        for row in results.get('rows'):
          lat = float(row[0])
          lon = float(row[1])
          country = row[2]
          city = row[3]
          page = row[4]
          f = math.pi / 180

    # Formula to convert a point expressed in (latitude, longitude) as degrees
    # to a point (x,y) expressed in centimeters.
    # This formula works with a planisphere based on Wagner VII projection
    # also known as Hammer-Wagner

          rad_lon = f * lon
          rad_lat = f * lat
          y = 0.90630778703664996 * math.sin(rad_lat)
          theta = math.asin(y)
          ct = math.cos(theta)
          lon_t = rad_lon / 3.0
          D = 1/(math.sqrt(0.5 * (1 + ct * math.cos(lon_t))))
          x = 2.66723 * ct * math.sin(lon_t)
          y *= 1.24104 * D
          x *= D
          x = round((x * 25.892745506), 2)
          y = round((y * 24.585971767), 2)

     # Check if we have a latitude and a longitude to move to
     # otherwise go standby and show tweets

          if (x != 0 and y != 0):
            print (x, y)
            output = country + ' / ' + city + ' / lat:' + format(lat, '.3f') + ' / lon:' + format(lon, '.3f')
            print(output)
            print(page.encode('ascii', 'ignore'))
          else:
            if ((time.time() - start) > 60):                                                                   
              print 'Getting tweets'                                                                           
              mentions = api.GetUserTimeline(screen_name='dot_cube',count=3)                                   
              start = time.time()
              i = 0                                                                              
           
            output = mentions[i%3].text.encode('ascii', 'ignore').replace('\n', ' ')
            page = '...'
            i += 1                                             
          try:
            value.put('page', page)
            value.put('geo', output)

            value.put('x', str(x-1))
            value.put('y', str(y))
          except socket.error:
            print 'Waiting Arduino...'
            time.sleep(10)
      else:
        print 'No Results Found'
    
        if ((time.time() - start) > 60):
          print 'Getting tweets'                                                                             
          mentions = api.GetUserTimeline(screen_name='dot_cube',count=3)
          start = time.time()
          i = 0

        output = mentions[i%3].text.encode('ascii', 'ignore').replace('\n', ' ')

        page = '...'
        i += 1
        try:    
          value.put('geo', output)
          value.put('page', page)
        except socket.error:                                                                             
          print 'Waiting Arduino...'                                                                     
          time.sleep(10)
     
      time.sleep(10)

    except TypeError, error:
  # Handle errors in constructing a query.
      print ('There was an error in constructing your query : %s' % error)

    except HttpError, error:
  # Handle API errors.
      if error.resp.reason in ['userRateLimitExceeded', 'quotaExceeded']:
        time.sleep((2 ** n) + random.random())
      else:
        print ('Arg, there was an API error : %s : %s' %
            (error.resp.status, error._get_reason()))

  # Handle errors.
    except BadStatusLine, error:
      print ('There was an error : %s' % error)

if __name__ == '__main__':
  main(sys.argv)
