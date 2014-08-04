/**
 * @ngdoc function
 * @name phototrackApp.controller:UsersCtrl
 * @description
 * # UsersCtrl
 * Controller of the phototrackApp
 */
 angular.module('phototrackApp')

 .factory('UsersService', function($resource,$routeParams,$rootScope,$http) {
 	$http.defaults.useXDomain = true;
 	var UsersService = $resource( API_SERVER+'/users/:Uuid',
		{},	{
			update: { method: 'PUT' },
			save: 	{ method: 'POST' },
			remove: { method: 'DELETE' },
			query: {method: 'GET', isArray: true }
		}
	);

 	UsersService.prototype.update = function(cb) {
		return UsersService.update({Uuid: this.Uuid},
			angular.extend({}, this, {Uuid:undefined}), cb);
	};


 	return UsersService;
 })
 .controller('UsersCtrl', function ($scope,$rootScope,UsersService) {
 	 	$rootScope.curmenu='users';

 	$scope.users = [];
 	UsersService.query({},function(users,p2){
 		$scope.users = users;
 	});

 })
 .controller('AddUserCtrl', function ($scope,$rootScope,$location,UsersService) {
 	$rootScope.curmenu='users';
 	$scope.user = new UsersService();
	$scope.save = function() {
		UsersService.save($scope.user, function(user) {

			//$location.path('/task/edit/' + task.id);
			$location.path('/users');
		});
	};


 })
 .controller('EditUserCtrl', function ($scope,$rootScope,$routeParams,$location,UsersService) {
 	 	$rootScope.curmenu='users';

 	UsersService.get( { Uuid : $routeParams.Uuid }, function(user){
 		$scope.user = user;
 	});

 	$scope.save = function() {
		$scope.user.update(function() {
			$location.path('/users');
		});
	};

	$scope.destroy = function()
	{
		if(confirm('Are you sure you want to delete this item') && UsersService.remove( {Uuid:$scope.user.Uuid } )){
			$location.path('/users');
		} 
	};

 })
 ;
