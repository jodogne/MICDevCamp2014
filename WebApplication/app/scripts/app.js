/**
 * @ngdoc overview
 * @name phototrackApp
 * @description
 * # phototrackApp
 *
 * Main module of the application.
 */


/* FOR TESTING */
var API_PATH =  'http://192.169.1.106:8001';
var API_SERVER = 'http://localhost:9292/192.169.1.106:8001';
//var API_SERVER = 'http://192.169.1.101:8000';


/* FOR PRODUCTION */
// var API_PATH = '/phototrackserver';
// var API_SERVER = '/phototrackserver';

angular
  .module('phototrackApp', [
    'ngAnimate',
    'ngCookies',
    'ngResource',
    'ngRoute',
    'ngSanitize'
  ])
  .config(function ($routeProvider) {
    $routeProvider
      .when('/', {
        templateUrl: 'views/main.html',
        controller: 'MainCtrl'
      })
      .when('/about', {
        templateUrl: 'views/about.html',
        controller: 'AboutCtrl'
      })

    // USERS  
      .when('/users', {
        templateUrl: 'views/users/list.html',
        controller: 'UsersCtrl'
      })
      .when('/user/add', {
        templateUrl: 'views/users/edit.html',
        controller: 'AddUserCtrl'
      })
      .when('/user/:Uuid', {
        templateUrl: 'views/users/edit.html',
        controller: 'EditUserCtrl'
      })
    // SITES
    .when('/sites', {
        templateUrl: 'views/sites/list.html',
        controller: 'SitesCtrl'
      })
   	.when('/site/add', {
        templateUrl: 'views/sites/edit.html',
        controller: 'AddSiteCtrl'
      })
   	.when('/site/:Uuid', {
        templateUrl: 'views/sites/edit.html',
        controller: 'EditSiteCtrl'
      })
   	// Photos
   	.when('/site/:Uuid/photos', {
        templateUrl: 'views/photos/list.html',
        controller: 'PhotosCtrl'
    })
    .when('/site/:Uuid/photos/add', {
        templateUrl: 'views/photos/edit.html',
        controller: 'AddPhotosCtrl'
    })
    .otherwise({
        redirectTo: '/'
      });
  });
