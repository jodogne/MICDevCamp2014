import re
import unittest
from RestToolbox import SetCredentials, DoGet, DoDelete, DoPut, DoPost


SESSIONS_URI = 'http://localhost:8000/sessions'
DEFAULT_LOGIN = {'username': 'user', 'password': 'pass'}
UUID_TEMPLATE = re.compile('^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$')

def IsValidUuid(uuid):
    return UUID_TEMPLATE.match(uuid) is not None

class SessionsTests(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        for session in DoGet(SESSIONS_URI):
            DoDelete(SESSIONS_URI + '/' + session)
    def setUp(self):
        self.sessionsUri = SESSIONS_URI
        self.defaultLogin = DEFAULT_LOGIN
    def testGetSessionsWhenEmpty(self):
        r = DoGet(self.sessionsUri)
        self.assertEqual(r, [])
    def testCreateNewSessionReturnsValidSession(self):
        r = DoPost(self.sessionsUri, self.defaultLogin)
        self.assertTrue(IsValidUuid(r['SessionId']))
        DoDelete(self.sessionsUri + '/' + r['SessionId'])
    def testDeleteSessionsMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete,
                                self.sessionsUri)
    def testPutSessionsMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.sessionsUri,
                                self.defaultLogin)

class SessionsUuidTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.sessionId = DoPost(SESSIONS_URI, DEFAULT_LOGIN)['SessionId']
    @classmethod
    def tearDownClass(cls):
        DoDelete(SESSIONS_URI + '/' + cls.sessionId)
    def setUp(self):
        self.sessionsUri = SESSIONS_URI
        self.defaultLogin = DEFAULT_LOGIN
        self.sessionIdUri = SESSIONS_URI + '/' + self.sessionId
    def testGetValidSessionId(self):
        r = DoGet(self.sessionIdUri)
        self.assertEqual(r, ['username'])
    def testGetInvalidSessionId(self):
        invalidSessionIdUri = SESSIONS_URI + '/12345678-90ab-cdef-1234-567890abcdef'
        self.assertRaisesRegexp(Exception, '400', DoGet, invalidSessionIdUri)
    def testDeleteSessionId(self):
        sessionId = DoPost(SESSIONS_URI, DEFAULT_LOGIN)['SessionId']
        r = DoDelete(SESSIONS_URI + '/' + sessionId)
        self.assertEqual(r, {})
        r = DoGet(SESSIONS_URI)
        self.assertEqual(r, [self.sessionId])
    def testPostSessionIdMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.sessionIdUri,
                                {'param': 'someparam', 'value': 'somevalue'})
    def testPutSessionIdMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.sessionIdUri,
                                self.defaultLogin)

class SessionsUuidUsernameTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.sessionId = DoPost(SESSIONS_URI, DEFAULT_LOGIN)['SessionId']
    @classmethod
    def tearDownClass(cls):
        DoDelete(SESSIONS_URI + '/' + cls.sessionId)
    def setUp(self):
        self.sessionsUri = SESSIONS_URI
        self.defaultLogin = DEFAULT_LOGIN
        self.sessionIdUri = SESSIONS_URI + '/' + self.sessionId
        self.sessionIdUsername = self.sessionIdUri + '/username'
    def testGetSessionIdUsername(self):
        r = DoGet(self.sessionIdUri + '/username')
        self.assertEqual(r, DEFAULT_LOGIN['username'])
    def testGetSessionIdInvalidPropertyFails(self):
        self.assertRaisesRegexp(Exception, '400', DoGet,
                                self.sessionIdUri + '/unknown')
    def testDeleteSessionIdUsernameMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.sessionIdUsername)
    def testPostSessionIdMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.sessionIdUri,
                                {'param': 'someparam', 'value': 'somevalue'})
    def testPutSessionIdMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.sessionIdUri,
                                self.defaultLogin)

def main():
    unittest.main()

if __name__ == '__main__':
    main()
