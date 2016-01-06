var DrawLatLonMarkers = function()
{
  var map = {}
  var layer_mapnik = {}
  var layer_markers = {}
  var init_map_ = function(lat, lon, zoom)
  {
    map = new OpenLayers.Map("map",
    {
        maxExtent: new OpenLayers.Bounds(-20037508.34,-20037508.34,20037508.34,20037508.34),
        maxResolution: 156543.0399,
        numZoomLevels: 19,
        units: 'm',
        projection: new OpenLayers.Projection("EPSG:900913"),
        displayProjection: new OpenLayers.Projection("EPSG:4326")
    });

    layer_mapnik = new OpenLayers.Layer.OSM.Mapnik("Mapnik");
    map.addLayer(layer_mapnik);

    var lonLat = new OpenLayers.LonLat(lon, lat).transform(new OpenLayers.Projection("EPSG:4326"), new OpenLayers.Projection("EPSG:900913"));

    map.setCenter(lonLat, zoom);
            
    layer_markers = new OpenLayers.Layer.Vector("Markers");
    map.addLayer(layer_markers);

    return map;
  }
  
  
  var point_style = OpenLayers.Util.extend({}, OpenLayers.Feature.Vector.style['default']);
  point_style.strokeColor = "blue";
  point_style.fillColor = "blue";
  var draw_point_ = function(lat, lon)
  {
    // create a point feature
    var point = new OpenLayers.Geometry.Point(lon, lat)
        .transform(new OpenLayers.Projection("EPSG:4326"), new OpenLayers.Projection("EPSG:900913"));
    var feature = new OpenLayers.Feature.Vector(point, null, point_style);            
    layer_markers.addFeatures([feature]);
  }
  
  
            
  var trace_style =
  {
      strokeColor: "blue",
      strokeOpacity: 1,
      strokeWidth: 3,
      pointRadius: 6,
      pointerEvents: "visiblePainted"
  };
  var draw_trace_ = function(trace)
  {
    var points = [];
    for (var i = 0; i < trace.length; ++i)
    {
      if (trace[i] && trace[i].lon && trace[i].lat)
      {
        var point = new OpenLayers.Geometry.Point(trace[i].lon, trace[i].lat)
            .transform(new OpenLayers.Projection("EPSG:4326"), new OpenLayers.Projection("EPSG:900913"));
        points.push(point);
      }
    }
    feature = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.LineString(points), null, trace_style);
    layer_markers.addFeatures([feature]);
  }
  
  
  var adjust_map_ = function()
  {
    extent = layer_markers.getDataExtent();
    if (extent)
      map.zoomToExtent(extent);
  }
  
  
  var i = 0;
  var dispenser_ = function() { return ++i; }
  
  
  var result =
  {
    init_map: init_map_,
    draw_point: draw_point_,
    draw_trace: draw_trace_,
    adjust_map: adjust_map_,
    dispenser: dispenser_
  };
  return result;
}();
