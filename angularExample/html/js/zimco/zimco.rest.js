/**
 * Created by Igor Mladenovic (mligor) on 23/12/13.
 */

(function(angular) 
{
'use strict';
angular.module('zimco.rest', ['ngResource'])

// RESTInterceptor
.factory('RESTInterceptor', ['$q', '$rootScope', function ($q, $rootScope)
{
	return {
		'response':function (response) 
		{
			if (response.status === 401) // Unauthorized
				$rootScope.$broadcast('Event:LoginRequired');
			else if (response.status < 200 && response.status >= 300) // Error
				return $q.reject(response);

			return response.data || $q.when(response.data);
		},
		'responseError': function (rejection) 
		{
			return $q.reject(rejection);
		}
	};
}])

// REST
.factory('REST', ['$resource', 'RESTInterceptor', function ($resource, RESTInterceptor)
{
	return function (url, paramDefaults, actions) {
		for (var key in actions)
		{
			if (actions.hasOwnProperty(key))
				actions[key].interceptor = RESTInterceptor;
		}
		return $resource(url, paramDefaults, actions);
	};
}])

/// SIMULATION

// REST_SIMULATIONInterceptor
.factory('REST_SIMULATIONInterceptor', ['$q', '$rootScope', function ($q, $rootScope)
{
	return {
		'response':function (response) 
		{
			response.data = response.config.simulation_fn(response.config.method, response.config.simulation_url, response.config);

			//console.log("sim-response", response);
			if (response.status === 401) // Unauthorized
				$rootScope.$broadcast('Event:LoginRequired');
			else if (response.status < 200 && response.status >= 300) // Error
				return $q.reject(response);

			return response.data || $q.when(response.data);
		},
		'responseError': function (rejection) 
		{
			//console.log("response err", rejection);
			return $q.reject(rejection);
		}
	};
}])

// REST_SIMULATION
.factory('REST_SIMULATION', ['$resource', 'REST_SIMULATIONInterceptor', function ($resource, RESTInterceptor)
{
	return function (fn, paramDefaults, actions) {
		for (var key in actions)
		{
			if (actions.hasOwnProperty(key))
			{
				actions[key].interceptor = RESTInterceptor;
				actions[key].simulation_fn = fn;
				actions[key].simulation_url = actions[key].url;
				if (actions[key].isArray)
					actions[key].url = "data:,[]";
				else
					actions[key].url = "data:,{}";
			}
		}
		return $resource('', paramDefaults, actions);
	};
}])
;

})(angular);
