var chartData4;

$(function(){
  $.ajax({

    url: '/testJm4',
    type: 'GET',
    success : function(data) {
      chartData4 = data;
      var template = Handlebars.compile($("#tabular-template").html());
      $("#table-location").html(template(data));

      var categoriesArray = [{
          "category" : data["categories"]
      }];

      if (chartData4["nodeNum"] == 4) {
      var myChart = new FusionCharts({
        type: 'realtimeline',
        dataFormat: 'json',
        id: 'stockMonitor',
        renderAt: 'chart'+ data["nodeNum"] +'-location',
        width: '550',
        height: '330',
        dataSource: {
            "chart": {
                "caption": "LAST GRAPH",
                "captionFontSize": "14",
                "subcaptionFontSize": "14",
                "baseFontColor" : "#333333",
                "baseFont" : "Helvetica Neue,Arial",                        
                "subcaptionFontBold": "0",
                "paletteColors" : "#f51200,#101def,#00f205",
                "bgColor" : "#ffffff",
                "canvasBgColor" : "#ffffff",
                "showBorder" : "0",
                "showShadow" : "0",
                "showCanvasBorder": "0",
                "showRealTimeValue": "0",
                "legendBorderAlpha": "0",
                "legendShadow": "0",
                "numbersuffix": "",
                "setadaptiveymin": "1",
                "setadaptivesymin": "1",
                "xAxisname": "Time",
                "yAxisname" : "Value",
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
                "showHoverEffect" : "1",
                "numDisplaySets": "20"
            },
            "categories": categoriesArray,
            "dataset": data["dataset"]
        },
        "events": {
            "initialized": function (e) {
                function formatTime(num) {
                    return (num <= 9)? ("0"+num) : num;
                }
                function updateData() {
                    // Get reference to the chart using its ID
                    var chartRef = FusionCharts("stockMonitor"),
                        //We need to create a querystring format incremental update, containing
                        //label in hh:mm:ss format
                        //and a value (random).
                        currDate = new Date(),
                        v1 = chartData4["value1"],
                        v2 = chartData4["value2"],
                        v3 = chartData4["value3"],
                        label = formatTime(currDate.getHours()) + ":" + formatTime(currDate.getMinutes()) + ":" + formatTime(currDate.getSeconds()),
                        //Build Data String in format &label=...&value=...
                        strData = "&label=" + chartData4["time"] + "&value=" + v1 + "|" + v2 + "|" + v3;
                        //strData =  "&label=" + label + "&value=" + str.data;
                    
                    //Feed it to chart.
                    chartRef.feedData(strData);
                }
                var myVar = setInterval(function () {
                  $(function(){
                    $.ajax({
                      url: '/testJm4',
                      type: 'GET',
                      success : function(data) {
                        chartData4 = data;
                        var template = Handlebars.compile($("#tabular-template").html());
                        $("#table-location").html(template(data));
                      }
                    });
                  });

                  updateData();
                }, 3000);
            }
        }
      }).render();}
    }


  }); // ajax end

}); //function end