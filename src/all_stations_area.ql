[out:csv("name";false)];
area[name~"{{area}}",i]->.a;
node(area.a)[name~"{{name}}",i][railway~"^(stop|halt|station)$"];
out;
