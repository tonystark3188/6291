 function dmui_ListView(){ 
   var listView = new Object; 
   listView.size = 0;
   listView.html = "<div id=\"_dmui_listview\"></div>";
   listView.getHtml = function(){ 
    return listView.html;
   }; 
   return listView; 
  } 

