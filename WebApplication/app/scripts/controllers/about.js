'use strict';

/**
 * @ngdoc function
 * @name phototrackApp.controller:AboutCtrl
 * @description
 * # AboutCtrl
 * Controller of the phototrackApp
 */
angular.module('phototrackApp')
  .controller('AboutCtrl', function ($scope,$rootScope) {
  	$rootScope.curmenu='about';
    $scope.awesomeThings = [
      'HTML5 Boilerplate',
      'AngularJS',
      'Karma'
    ];
  });
