'use strict';

/**
 * @ngdoc function
 * @name phototrackApp.controller:MainCtrl
 * @description
 * # MainCtrl
 * Controller of the phototrackApp
 */
angular.module('phototrackApp')
  .controller('MainCtrl', function ($scope,$rootScope) {
  	$rootScope.curmenu='home';
    $scope.awesomeThings = [
      'HTML5 Boilerplate',
      'AngularJS',
      'Karma'
    ];
  });
