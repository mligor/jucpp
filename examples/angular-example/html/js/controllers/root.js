(function(angular) {
'use strict';

angular.module('app').controller('RootController', ['$scope', 'User', 'Feed',
function($scope, User, Feed)
{
    $scope.users = [];
    $scope.feeds = [];
    
    $scope.selected = null;
    $scope.fselected = null;
    
    var init = function()
    {
        console.log("init()");
        User.get().then(
            function onSuccess(userList)
            {
                $scope.users = userList;
            },
            function onError(err)
            {
                alert(err);
            });
            
        Feed.get().then(
            function onSuccess(feeds)
            {
                $scope.feeds = feeds;
            },
            function onError(err)
            {
                alert(err);
            });

    };
    
    $scope.select = function(u)
    {
        $scope.selected = u;
    }
    
    $scope.addUser = function()
    {
        console.log("addUser()");
        User.add({"firstName": "Igor", "lastName" : "Mladenovic", email: "test@test.com"}).then(
            function onSuccess(newUser)
            {
                $scope.users.push(newUser);
            },
            function onError(err)
            {
                alert(err);
            });
    }
    
    $scope.addFeed = function()
    {
        console.log("addUser()");
        Feed.add({"url" : "http://myurl", title: "Dummy feed"}).then(
        function onSuccess(newFeed)
        {
            $scope.feeds.push(newFeed);
        },
        function onError(err)
        {
            alert(err);
        });
    }
    
    $scope.editUser = function(user)
    {
        console.log("editUser()", user);
        user["lastEdit"] = new Date();
        User.write(user).then(
        function onSuccess(newUser)
            {
                init();
            },
            function onError(err)
            {
                alert(err);
            });
    }
    
    $scope.deleteUser = function(user)
    {
        console.log("deleteUser()");
        User.delete(user).then(
            function onSuccess()
            {
                init();
                //$scope.users.push(newUser);
            },
            function onError(err)
            {
                alert(err);
            });

    }
    
    
    
    init();
    
}]);
})(angular);