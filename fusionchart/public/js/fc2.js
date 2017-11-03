var chartData2;

$(function(){
  $.ajax({

    url: '/testJm2',
    type: 'GET',
    success : function(data) {
      chartData2 = data;
      var template = Handlebars.compile($("#tabular-template").html());
      $("#table-location").html(template(data));

      var categoriesArray = [{
          "category" : data["categories"]
      }];

      if (chartData2["nodeNum"] == 2) {
      var myChart = new FusionCharts({
        type: 'realtimeline',
        dataFormat: 'json',
        id: 'node2chart',
        renderAt: 'chart2-location',
        width: '550',
        height: '330',
        dataSource: {
            "chart": {
                "caption": "SPEED",
                "captionFontSize": "14",
                "subcaptionFontSize": "14",
                "baseFontColor" : "#333333",
                "baseFont" : "Helvetica Neue,Arial",                        
                "subcaptionFontBold": "0",
                "paletteColors" : "#0075c2,#1aaf5d,#f2c500",
                "bgColor" : "#ffffff",
                "canvasBgColor" : "#ffffff",
                "showBorder" : "0",
                "showShadow" : "0",
                "showCanvasBorder": "0",
                "showRealTimeValue": "0",
                "legendBorderAlpha": "0",
                "legendShadow": "0",
                "numberprefix": "",
                "setadaptiveymin": "1",
                "setadaptivesymin": "1",
                "xaxisname": "Time",
                "labeldisplay": "Rotate",
                "slantlabels": "1",
                "pyaxisminvalue": "35",
                "pyaxismaxvalue": "36",
                "divlineAlpha" : "100",
                "divlineColor" : "#999999",
                "showAlternateHGridColor" : "0",
                "divlineThickness" : "1",
                "divLineIsDashed" : "1",
                "divLineDashLen" : "1",
                "divLineGapLen" : "1",
                "numDisplaySets": "10"
            },
            "categories": categoriesArray,
            "dataset": data["dataset"]
        },
        "events": {
            "initialized": function (e) {
                function updateData() {
                    // Get reference to the chart using its ID
                    var chartRef = FusionCharts("node2chart"),
                        currDate = new Date(),
                        v1 = chartData2["value1"],
                        v2 = chartData2["value2"],
                        v3 = chartData2["value3"],
                        //**데이터베이스로부터 값을 읽어와서 업데이트 시켜줘야함**
                        strData = "&label=" + chartData2["time"] + "&value=" + v1 + "|" + v2 + "|" + v3;
                        //strData =  "&label=" + label + "&value=" + str.data;
                    
                    //Feed it to chart.
                    chartRef.feedData(strData);
                    console.log(nyse);
                }
                var myVar = setInterval(function () {
                    updateData();
                }, 3000);
            }
        }
      }).render();}
    }


  }); // ajax end


}); //function end

var chart2update = setInterval(function(){
  $(function(){
  $.ajax({
    url: '/testJm2',
    type: 'GET',
    success : function(data) {
      chartData2 = data;
      var template = Handlebars.compile($("#tabular-template").html());
      $("#table-location").html(template(data));
    }
});
});
},3000);