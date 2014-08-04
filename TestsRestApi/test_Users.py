import re
import unittest
from RestToolbox import SetCredentials, DoGet, DoDelete, DoPut, DoPost


USERS_URI = 'http://localhost:8000/users'
DEFAULT_USERDATA = {
    'FullName': 'Joe User',
    'Email': 'user@domain.com',
    'Organization': 'SuperOrg',
    'UserName': 'usr',
    'Password': 'pwd',
    'IsAdmin': True,
    'IsSupervisor': True,
}
UUID_TEMPLATE = re.compile('^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$')

def IsValidUuid(uuid):
    return UUID_TEMPLATE.match(uuid) is not None

class UsersTests(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        for user in DoGet(USERS_URI):
            DoDelete(USERS_URI + '/' + user['Uuid'])
    def setUp(self):
        self.usersUri = USERS_URI
        self.userData = DEFAULT_USERDATA
    def testGetUsersWhenEmpty(self):
        r = DoGet(self.usersUri)
        self.assertEqual(r, [])
    def testCreateNewUserReturnsValidUser(self):
        r = DoPost(self.usersUri, self.userData)
        self.assertTrue(IsValidUuid(r['UserId']))
        userData = DoGet(self.usersUri + '/' + r['UserId'])
        DoDelete(USERS_URI + '/' + r['UserId'])
        for key in self.userData.keys():
            self.assertEqual(userData[key], self.userData[key])
    def testDeleteUsersMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete,
                                self.usersUri)
    def testPutUsersMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.usersUri,
                                self.userData)

class UsersUuidTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.userId = DoPost(USERS_URI, DEFAULT_USERDATA)['UserId']
    @classmethod
    def tearDownClass(cls):
        DoDelete(USERS_URI + '/' + cls.userId)
    def setUp(self):
        self.usersUri = USERS_URI
        self.userData = DEFAULT_USERDATA
        self.userIdUri = USERS_URI + '/' + self.userId
    def testGetValidUserId(self):
        userData = DoGet(self.userIdUri)
        for key in self.userData.keys():
            self.assertEqual(userData[key], self.userData[key])
    def testGetInvalidUserId(self):
        invalidUserIdUri = USERS_URI + '/12345678-90ab-cdef-1234-567890abcdef'
        self.assertRaisesRegexp(Exception, '400', DoGet, invalidUserIdUri)
    def testUpdateUserId(self):
        updatedUserData = {
            'Uuid': self.userId,
            'FullName': 'Jack Reacher',
            'Email': 'jack@reacher.com',
            'Organization': 'Killer Inc.',
            'UserName': 'jack',
            'Password': 'reacher123',
            'IsAdmin': False,
            'IsSupervisor': False,
        }
        previousUserData = DoGet(self.userIdUri)
        DoPut(self.userIdUri, updatedUserData)
        userData = DoGet(self.userIdUri)
        DoPut(self.userIdUri, previousUserData)
        for key in userData.keys():
            self.assertEqual(userData[key], updatedUserData[key])
    def testDeleteUserId(self):
        userId = DoPost(USERS_URI, DEFAULT_USERDATA)['UserId']
        r = DoDelete(USERS_URI + '/' + userId)
        self.assertEqual(r, {})
        r = DoGet(USERS_URI)
        self.assertEqual(len(r), 1)
        for key in self.userData.keys():
            self.assertEqual(r[0][key], self.userData[key])
    def testPostUserIdMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdUri,
                                {'param': 'someparam', 'value': 'somevalue'})

class UsersUuidDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.userId = DoPost(USERS_URI, DEFAULT_USERDATA)['UserId']
        cls.userIdUri = USERS_URI + '/' + cls.userId
    @classmethod
    def tearDownClass(cls):
        DoDelete(cls.userIdUri)
    def setUp(self):
        self.usersUri = USERS_URI
        self.userData = DEFAULT_USERDATA
        self.userIdUri = USERS_URI + '/' + self.userId
        self.userIdEmailUri = self.userIdUri + '/email'
        self.userIdFullNameUri = self.userIdUri + '/full-name'
        self.userIdIsAdminUri = self.userIdUri + '/is-admin'
        self.userIdIsSupervisorUri = self.userIdUri + '/is-supervisor'
        self.userIdOrganizationUri = self.userIdUri + '/organization'
        self.userIdPasswordUri = self.userIdUri + '/password'
        self.userIdUserNameUri = self.userIdUri + '/username'
    def testGetEmail(self):
        email = DoGet(self.userIdEmailUri)
        self.assertEqual(email, self.userData['Email'])
    def testPostEmailMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdEmailUri,
                                'test@email.com')
    def testDeleteEmailMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.userIdEmailUri)
    def testPutEmailMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.userIdEmailUri,
                                'new@email.com')
    def testGetFullName(self):
        fullName = DoGet(self.userIdFullNameUri)
        self.assertEqual(fullName, self.userData['FullName'])
    def testPostFullNameMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdFullNameUri,
                                'Jack Reacher')
    def testDeleteFullNameMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.userIdFullNameUri)
    def testPutFullNameMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.userIdFullNameUri,
                                'Tony Ryan')
    def testGetIsAdmin(self):
        admin = DoGet(self.userIdIsAdminUri)
        self.assertEqual(admin, self.userData['IsAdmin'])
    def testPostIsAdminMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdIsAdminUri,
                                True)
    def testDeleteIsAdminMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.userIdIsAdminUri)
    def testPutIsAdminMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.userIdIsAdminUri,
                                False)
    def testGetIsSupervisor(self):
        supervisor = DoGet(self.userIdIsSupervisorUri)
        self.assertEqual(supervisor, self.userData['IsAdmin'])
    def testPostIsSupervisorMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdIsSupervisorUri,
                                True)
    def testDeleteIsSupervisorMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.userIdIsSupervisorUri)
    def testPutIsSupervisorMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.userIdIsSupervisorUri,
                                False)
    def testGetOrganization(self):
        organization = DoGet(self.userIdOrganizationUri)
        self.assertEqual(organization, self.userData['Organization'])
    def testPostOrganizationMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdOrganizationUri,
                                'ACME')
    def testDeleteOrganizationMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.userIdOrganizationUri)
    def testPutOrganizationMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.userIdOrganizationUri,
                                'Global Corp')
    def testGetUserName(self):
        userName = DoGet(self.userIdUserNameUri)
        self.assertEqual(userName, self.userData['UserName'])
    def testPostUserNameMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdUserNameUri,
                                'reacherj')
    def testDeleteUserNameMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.userIdUserNameUri)
    def testPutUserNameMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.userIdUserNameUri,
                                'ryant')
    def testGetPassword(self):
        password = DoGet(self.userIdPasswordUri)
        self.assertEqual(password, self.userData['Password'])
    def testPostPasswordMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPost, self.userIdPasswordUri,
                                'reacher123')
    def testDeletePasswordMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoDelete, self.userIdPasswordUri)
    def testPutPasswordMustFail(self):
        self.assertRaisesRegexp(Exception, '405', DoPut, self.userIdPasswordUri,
                                'ryan456')

def main():
    unittest.main()

if __name__ == '__main__':
    main()
