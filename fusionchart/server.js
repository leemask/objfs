//import express package
var express = require("express");
//import mysql package
var mysql = require("mysql");
var http = require("http");
var path = require("path");
var bodyParser = require("body-parser");
var fusioncharts = require("fusioncharts");


//mysql db server connection
var connection = mysql.createConnection({
  host : 'localhost',
  user : 'root',
  password : 'flash770',
  port : 3306,
  database : 'testdb'
});
connection.connect();

//create express app
var app = express();
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({extended : true}));

var node1 = 0;
var node2 = 0;
var node3 = 0;
var node4 = 0;
var value;

app.post('/node', function(req, res) {
  if (!req.body.node1) {
    node1 = 0;
  } else {
    node1 = req.body.node1;
  }
  if (!req.body.node2) {
    node2 = 0;
  } else {
    node2 = req.body.node2;
  }
  if (!req.body.node3) {
    node3 = 0;
  } else {
    node3 = req.body.node3;
  }
  if (!req.body.node4) {
    node4 = 0;
  } else {
    node4 = req.body.node4;
  }

  res.send("<meta http-equiv='refresh' content='0; url=/'>");
});

var recieve_node = 0;
app.post('/testJm', function(req, res) {
	recieve_node = 1;
	getArduino(req,res);
});
app.post('/testJm2', function(req, res) {
	recieve_node = 2;
	getArduino(req,res);
});
app.post('/testJm3', function(req, res) {
	recieve_node = 3;
	getArduino(req,res);
});
app.post('/testJm4', function(req, res) {
	recieve_node = 4;
	getArduino(req,res);
});

function getArduino(req, res) {
  if (req.body.data) {
    value = req.body.data;
  }
    
    console.log("target : testJM" + recieve_node + " value : " + value);
  
    connection.query('INSERT into test values(curdate(), curtime(), "'+ value +'", null, null, ' + recieve_node + ')');

}

function updateData() {
  // Get reference to the chart using its ID
  var chartRef = getChartFromId("stockMonitor"),
  //We need to create a querystring format incremental update, containing
  //label in hh:mm:ss format
  //and a value (random).
  currDate = new Date(),
  v1 = 1,
  v2 = 2,
  v3 = 3,
  label = formatTime(currDate.getHours()) + ":" + formatTime(currDate.getMinutes()) + ":" + formatTime(currDate.getSeconds()),
  //Build Data String in format &label=...&value=...
  strData = "&label=" + label + "&value=" + v1 + "|" + v2 + "|" + v3;
  //strData =  "&label=" + label + "&value=" + str.data;
                    
  //Feed it to chart.
  //chartRef.feedData(strData);
}

function formatTime(num) {
  return (num <= 9)? ("0"+num) : num;
}

function getData(responseObj, node){ 

  //use the find() API and pass an empty query object to retrieve all records
  connection.query('SELECT * from test where node = "'+ node +'" order by date desc, time desc limit 1', function(err, rows, fields) {
    if (!err){
      var timeArray = [];
      var value1Array = [];
      var value2Array = [];
      var value3Array = [];
      var value1;
      var value2;
      var value3;
      var time;
      var nodeNum = node;

      rows.forEach(function(d){
        time = d.time;
        value1 = d.value1;
/*        value2 = d.value2;
        value3 = d.value3;
*/        timeArray.push({"label" : time});
        value1Array.push({"value" : value1});
/*        value2Array.push({"value" : value2});
        value3Array.push({"value" : value3});
*/      })

      var dataset = [
      {
        "seriesname" : "Test",
        "data" : value1Array
      }/*,
      {
        "seriesname" : "Value2",
        "data" : value2Array
      }, 
      {
        "seriesname" : "Value3",
        "data" : value3Array
      }*/
      ];


      var response = {
        "dataset" : dataset,
        "categories" : timeArray,
        "time" : time,
        "value1" : value1,
/*      "value2" : value2,
        "value3" : value3,
*/       "nodeNum" : nodeNum
      };

      responseObj.json(response);
    } else {
      console.log('Error while performing Query.', err);
    }
    
    });
    
   // connection.end();
} //getData end



//NPM Module to integrate Handlerbars UI template engine with Express
var exphbs  = require('express-handlebars');

//Declaring Express to use Handlerbars template engine with main.handlebars as
//the default layout
app.engine('handlebars', exphbs({defaultLayout: 'main'}));
app.set('view engine', 'handlebars');

//Defining middleware to serve static files
app.use('/public', express.static('public'));


app.get("/testJm", function(req, res) {
  getData(res, node1);
});

app.get("/testJm2", function(req, res) {
  getData(res, node2);
});

app.get("/testJm3", function(req, res) {
  getData(res, node3);
});

app.get("/testJm4", function(req, res) {
  getData(res, node4);
});


app.get("/", function(req, res) {
  res.render("chart");
});

app.listen("3300", function(){
  console.log('Server up: http://localhost:3300');
});
