import re
import unittest
from RestToolbox import SetCredentials, DoGet, DoDelete, DoPut, DoPost


SERVER_URI = 'http://localhost:8000'
SITES_URI = SERVER_URI + '/sites'
PHOTOS_URI = SERVER_URI + '/photos'
DEFAULT_PHOTODATA = {
    'Image': 'BeautifulImage',
    'Latitude': 50.5,
    'Longitude': 5.7,
    'SecondsSinceEpoch': '999999999',
    'Tag': 'Yeah!',
    'SiteUuid': '12345678-90ab-cdef-1234-567890abcdef',
}
DEFAULT_SITEDATA = {
    'Address': 'Tchernobyl, Ukraine',
    'Archive': 'myarchive.zip',
    'Latitude': 50.8,
    'Longitude': 5.9,
    'Name': 'The Site',
    'Photos': [DEFAULT_PHOTODATA],
    'PitNumber': 'Le Trou #4',
    'SecondsSinceEpoch': '666666666',
    'Status': 4,
}
UUID_TEMPLATE = re.compile('^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$')

def IsValidUuid(uuid):
    return UUID_TEMPLATE.match(uuid) is not None

class PhotosTests(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        for photo in DoGet(PHOTOS_URI):
            DoDelete(PHOTOS_URI + '/' + photo['Uuid'])
    def setUp(self):
        self.sitesUri = SITES_URI
        self.photosUri = PHOTOS_URI
        self.photoData = DEFAULT_PHOTODATA
    def testGetPhotosWhenEmpty(self):
        r = DoGet(self.photosUri)
        self.assertEqual(r, [])
    def testCreateNewPhotoReturnsValidPhoto(self):
        siteId = DoPost(SITES_URI, DEFAULT_SITEDATA)['SiteId']
        defaultPhotoData = DEFAULT_PHOTODATA.copy()
        defaultPhotoData['SiteUuid'] = siteId
        r = DoPost(self.photosUri, defaultPhotoData)
        self.assertTrue(IsValidUuid(r['PhotoId']))
        photoData = DoGet(self.photosUri + '/' + r['PhotoId'])
        DoDelete(SITES_URI + '/' + siteId)
        for key in defaultPhotoData.keys():
            if key not in ['Image', 'ImageMime', 'Uuid', 'Time']:
                expected = defaultPhotoData[key]
                if isinstance(expected, float):
                    value = round(photoData[key], 1)
                else:
                    value = photoData[key]
                self.assertEqual(value, expected)
        self.assertTrue(IsValidUuid(photoData['Uuid']))
        self.assertTrue('ImageMime' in photoData)
        self.assertTrue('Time' in photoData)
    def testDeletePhotosMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete,
                                self.photosUri)
    def testPutPhotosMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.photosUri,
                                self.photoData)

def main():
    unittest.main()

if __name__ == '__main__':
    main()
