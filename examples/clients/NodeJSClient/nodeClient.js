

var request = require('request')
var url = 'http://10.10.2.4:1000/ON' // input your url here /ON to turn on and /OFF to turn off

// use a timeout value of 10 seconds
var timeoutInMilliseconds = 10*1000
var opts = {
  url: url,
  timeout: timeoutInMilliseconds
}

request(opts, function (err, res, body) {
  if (err) {
    console.dir(err)
    return
  }
  var statusCode = res.statusCode
  console.log('status code: ' + statusCode)
})