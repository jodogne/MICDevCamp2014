/**
 * @ngdoc function
 * @name phototrackApp.controller:SitesCtrl
 * @description
 * # SitesCtrl
 * Controller of the phototrackApp
 */
 angular.module('phototrackApp')


.factory('PhotosService', function($resource,$routeParams,$rootScope,$http) {
 	$http.defaults.useXDomain = true;
 
 	var PhotosService = $resource( API_SERVER+'/photos/:Uuid',
		{},	{
			update: { method: 'PUT' },
			save: 	{ method: 'POST' },
			remove: { method: 'DELETE' },
			query: {method: 'GET', isArray: true }
		}
	);

 	return PhotosService;
})


.factory('PhotosSitesService', function($resource,$routeParams,$rootScope,$http) {
 	$http.defaults.useXDomain = true;
 
 	var PhotosSitesService = $resource( API_SERVER+'/sites/:Uuid/photos',
		{},	{
			
			query: {method: 'GET', isArray: true }
		}
	);

 	return PhotosSitesService;
})
 
.factory('SitesService', function($resource,$routeParams,$rootScope,$http) {
 	$http.defaults.useXDomain = true;
 
 	var SitesService = $resource( API_SERVER+'/sites/:Uuid',
		{},	{
			update: { method: 'PUT' },
			save: 	{ method: 'POST' },
			remove: { method: 'DELETE' },
			query: {method: 'GET', isArray: true }
		}
	);

 	SitesService.prototype.update = function(cb) {
 		this.Status = parseInt(this.Status);
		return SitesService.update({Uuid: this.Uuid},
			angular.extend({}, this, {Uuid:undefined}), cb);
	};


 	return SitesService;
})
.controller('SitesCtrl', function ($scope,$rootScope,SitesService) {
	$rootScope.curmenu='sites';
	$scope.list = [];
	$scope.API_SERVER = API_SERVER;
	$scope.API_PATH = API_PATH;
	SitesService.query({},function(data){
		$scope.list = data;
	});


})
.controller('AddSiteCtrl', function ($rootScope,$scope,$location,SitesService) {
	$scope.site = {};
	$rootScope.curmenu='sites';
	$scope.save = function() {
		$scope.site.Status = parseInt($scope.site.Status);
		SitesService.save($scope.site, function(site) {
			// console.log( site );
			//$location.path('/task/edit/' + task.id);
			$location.path('/sites');
		});
	};

	$scope.geocode = function(){

		geoCodeAddress($scope.site.Address).then(function(result, status){
				if (status === google.maps.GeocoderStatus.OK) {
				var latLng = {
					lat: result[0].geometry.location.lat(),
					lng: result[0].geometry.location.lng()
				};
				$scope.site.Latitude = latLng.lat;
				$scope.site.Longitude = latLng.lng;
				$scope.$apply();
			}
		});
		
		
	};

	$scope.geocodehere = function(){
		if(!!navigator.geolocation){

			navigator.geolocation.getCurrentPosition(function(position) {
	            geoCodeLatLng(position.coords.latitude, position.coords.longitude).then(function(result, status){
					if (status === google.maps.GeocoderStatus.OK) {
						$scope.site.Address = result[0].formatted_address;
						$scope.site.Latitude = position.coords.latitude;
						$scope.site.Longitude = position.coords.longitude;
						$scope.$apply();
					}
				});
	          
			});
		}
	};

	
})
.controller('EditSiteCtrl', function ($scope,$rootScope,$routeParams,$location,SitesService) {
	$rootScope.curmenu='sites';
	SitesService.get( { Uuid : $routeParams.Uuid }, function(site){
 		$scope.site = site;
 	});

 	$scope.save = function() {
		$scope.site.update(function() {
			$location.path('/sites');
		});
	};


	$scope.destroy = function()
	{
		if(confirm('Are you sure you want to delete this item')){
			SitesService.remove( {Uuid:$scope.site.Uuid } , function(result){

				$location.path('/sites');
			});
		} 
	};

	$scope.geocode = function(){

		geoCodeAddress($scope.site.Address).then(function(result, status){
				if (status === google.maps.GeocoderStatus.OK) {
				var latLng = {
					lat: result[0].geometry.location.lat(),
					lng: result[0].geometry.location.lng()
				};
				$scope.site.Latitude = latLng.lat;
				$scope.site.Longitude = latLng.lng;
				$scope.$apply();
			}
		});
		
		
	};

	$scope.geocodehere = function(){
		if(!!navigator.geolocation){

			navigator.geolocation.getCurrentPosition(function(position) {
	            geoCodeLatLng(position.coords.latitude, position.coords.longitude).then(function(result, status){
					if (status === google.maps.GeocoderStatus.OK) {
						$scope.site.Address = result[0].formatted_address;
						$scope.site.Latitude = position.coords.latitude;
						$scope.site.Longitude = position.coords.longitude;
						$scope.$apply();
					}
				});
	          
			});
		}
	};
})

.controller('PhotosCtrl', function($scope,$rootScope,$routeParams,$location,SitesService,PhotosService,PhotosSitesService){
	$scope.API_SERVER = API_SERVER;
	$scope.API_PATH = API_PATH;
	$rootScope.curmenu='sites';
	PhotosSitesService.query({Uuid: $routeParams.Uuid },function( data ){
		$scope.list = data;	
	});

	SitesService.get( { Uuid : $routeParams.Uuid }, function(site){
 		$scope.site = site;
 	});

	$scope.open = function(image){
		// console.log( image,'image' );
		$scope.curImg = API_PATH+'/photos/'+image.Uuid+'/image';
		$("#myModal").modal();
	};
})
.controller('AddPhotosCtrl', function($scope,$rootScope,$routeParams,$location,SitesService,PhotosService){
	$rootScope.curmenu='sites';
	$scope.list = PhotosService.list;

	$scope.open = function(image){
		// console.log( image,'image' );
		$.fancybox({href:image.url,title:image.description});
	};
})

;
