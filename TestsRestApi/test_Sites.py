import re
import unittest
from RestToolbox import SetCredentials, DoGet, DoDelete, DoPut, DoPost


SITES_URI = 'http://localhost:8000/sites'
FIRST_PHOTODATA = {
    'Image': 'BeautifulImage',
    'Latitude': 50.5,
    'Longitude': 5.7,
    'SecondsSinceEpoch': '999999999',
    'Tag': 'Yeah!',
    'SiteUuid': '12345678-90ab-cdef-1234-567890abcdef',
}
SECOND_PHOTODATA = {
    'Image': 'BeautifulImage',
    'Latitude': 50.5,
    'Longitude': 5.7,
    'SecondsSinceEpoch': '999999999',
    'Tag': 'Yeah!',
    'SiteUuid': '12345678-90ab-cdef-1234-567890abcdef',
}
DEFAULT_SITEDATA = {
    'Address': 'Tchernobyl, Ukraine',
    'Latitude': 50.8,
    'Longitude': 5.9,
    'Name': 'The Site',
    #'Photos': [FIRST_PHOTODATA, SECOND_PHOTODATA],
    'PitNumber': 'Le Trou #4',
    'SecondsSinceEpoch': '666666666',
    'Status': 4,
}

UUID_TEMPLATE = re.compile('^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$')

def IsValidUuid(uuid):
    return UUID_TEMPLATE.match(uuid) is not None

class SitesTests(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        for site in DoGet(SITES_URI):
            DoDelete(SITES_URI + '/' + site['Uuid'])
    def setUp(self):
        self.sitesUri = SITES_URI
        self.siteData = DEFAULT_SITEDATA
    def testGetSitesWhenEmpty(self):
        r = DoGet(self.sitesUri)
        self.assertEqual(r, [])
    def testCreateNewSiteReturnsValidSite(self):
        r = DoPost(self.sitesUri, self.siteData)
        self.assertTrue(IsValidUuid(r['SiteId']))
        siteData = DoGet(self.sitesUri + '/' + r['SiteId'])
        DoDelete(SITES_URI + '/' + r['SiteId'])
        for key in self.siteData.keys():
            expected = self.siteData[key]
            if isinstance(expected, float):
                value = round(siteData[key], 1)
            else:
                value = siteData[key]
            self.assertEqual(value, expected)
        self.assertTrue(IsValidUuid(siteData['Uuid']))
        self.assertTrue('Time' in siteData)
    def testDeleteSitesMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete,
                                self.sitesUri)
    def testPutSitesMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.sitesUri,
                                self.siteData)

class SitesUuidTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.siteId = DoPost(SITES_URI, DEFAULT_SITEDATA)['SiteId']
    @classmethod
    def tearDownClass(cls):
        DoDelete(SITES_URI + '/' + cls.siteId)
    def setUp(self):
        self.sitesUri = SITES_URI
        self.siteData = DEFAULT_SITEDATA
        self.siteIdUri = SITES_URI + '/' + self.siteId
    def testGetValidSiteId(self):
        siteData = DoGet(self.siteIdUri)
        for key in self.siteData.keys():
            expected = self.siteData[key]
            if isinstance(expected, float):
                value = round(siteData[key], 1)
            else:
                value =siteData[key]
            self.assertEqual(value, expected)
    def testGetInvalidSiteId(self):
        invalidSiteIdUri = SITES_URI + '/12345678-90ab-cdef-1234-567890abcdef'
        self.assertRaisesRegexp(Exception, '400', DoGet, invalidSiteIdUri)
    def testUpdateSiteId(self):
        updatedSiteData = {
            'Address': 'Moscow, Russia',
            'Latitude': 40.8,
            'Longitude': 35.9,
            'Name': 'Kremlin',
            'PitNumber': 'Le Trou #666',
            'SecondsSinceEpoch': '777777777',
            'Status': 5,
        }
        previousSiteData = DoGet(self.siteIdUri)
        DoPut(self.siteIdUri, updatedSiteData)
        siteData = DoGet(self.siteIdUri)
        DoPut(self.siteIdUri, previousSiteData)
        for key in updatedSiteData.keys():
            expected = updatedSiteData[key]
            if isinstance(expected, float):
                value = round(siteData[key], 1)
            else:
                value = siteData[key]
            self.assertEqual(value, expected)
        self.assertTrue(IsValidUuid(siteData['Uuid']))
        self.assertTrue('Time' in siteData)
    def testDeleteSiteId(self):
        siteId = DoPost(SITES_URI, DEFAULT_SITEDATA)['SiteId']
        r = DoDelete(SITES_URI + '/' + siteId)
        self.assertEqual(r, {})
        r = DoGet(SITES_URI)
        self.assertEqual(len(r), 1)
        for key in self.siteData.keys():
            expected = self.siteData[key]
            if isinstance(expected, float):
                value = round(r[0][key], 1)
            else:
                value = r[0][key]
            self.assertEqual(value, expected)
    def testPostSiteIdMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.siteIdUri,
                                {'param': 'someparam', 'value': 'somevalue'})


def main():
    unittest.main()

if __name__ == '__main__':
    main()
