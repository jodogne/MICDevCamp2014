
function geoCodeAddress(address)
{

	var deferred = $.Deferred();
    var geoCoder = new google.maps.Geocoder();
    geoCoder.geocode({ address : address }, deferred.resolve);
    return deferred.promise();
	
}


function geoCodeLatLng(lat,lng)
{
	var deferred = $.Deferred();

    var latlng = new google.maps.LatLng(lat, lng );
    var geoCoder = new google.maps.Geocoder();
    geoCoder.geocode({ location: latlng }, deferred.resolve);
    return deferred.promise();	
}
