


// function format_disk_progress()
// {
// 	var xml = json2xml({ "setSysInfo": { "GetFormatProcess": ""}});	

// 	//发出请求不一定会返回，有时候可以返回有时候不能，所以发送以后直接倒计时，提示连接成功。
// 	$.ajax({ url: appendTS(api.apiurl),
// 	    type: "POST",
// 	    data: { data: xml },
// 	    contentType: "application/x-www-form-urlencoded;",
// 	    success: function (result) {

// 	        //Loaded();
// 	        var result = $.xml2json(result);
// 	        if ((result.Return == undefined) || result.Return.status != "true")
// 	        {
// 	         	//alert(result.Return.toString());
// 	         	console.log("不支持或返回false");
// 	         	return;
// 	         }
	        

// 	        var progress = result.GetFormatProcess;


// 	    }
// 	});
// }

function showErrorToast()
{
	layer.msg('请求失败');
}

function hideLoadingView()
{
	$("#loading").hide();
}

function showLoadingView()
{
	$("#loading").show();
}

function hideWorkModeLoadingView()
{
	$("#workmode_loading").hide();
}

function showWorkModeLoadingView()
{
	$("#workmode_loading").show();
}

function format_disk()
{
		var xml = json2xml({ "setSysInfo": { "Format": ""}});	

	//发出请求不一定会返回，有时候可以返回有时候不能，所以发送以后直接倒计时，提示连接成功。
	$.ajax({ url: appendTS(api.apiurl),
	    type: "POST",
	    data: { data: xml },
	    contentType: "application/x-www-form-urlencoded;",
	    success: function (result) {
	    	hideLoadingView();
	        //Loaded();
	        var result = $.xml2json(result);
	        if(result.Return == undefined)
	        {
				console.log("格式化命令不支持");
				layer.msg("格式化失败");
	        	 //android
	        	window.LetvJSBridge_For_Android.callLeboxChangeFomatState(false);				
				return;
	        }

	        if ((result.Return != undefined)  && (result.Return.status == "false"))
	        {
	        	//alert(result.Return.toString());
	        	console.log("格式化失败");
	        	//提示格式化失败

	        	layer.msg("格式化失败");
	        	 //android
	        	window.LetvJSBridge_For_Android.callLeboxChangeFomatState(false);
	        	return;

	        } 
	        console.log("格式化命令请求成功！");
	        //format_disk_progress();
	        layer.msg("格式化成功",{time:1000});
	        //开始刷新界面
	        initView();

	        //android
	        window.LetvJSBridge_For_Android.callLeboxChangeFomatState(true);

	    },
	    error:function(x,e)
	    {
	    	//layer.msg("格式化失败");
	    	hideLoadingView();
	    	//android
	        window.LetvJSBridge_For_Android.callLeboxChangeFomatState(true);
	    }
	});

	showLoadingView();
}

function click_dialog_cancel()
{
	layer.closeAll();
}

function click_dialog_format()
{
	layer.closeAll();
//	alert("这里处理连接操作");
	console.log("click_dialog_format() 这里处理连接操作");
	//todo 
	format_disk();
}

function switch_workmode(mode)
{

	var attr = new Object();
	attr.mode = mode;
		var xml = json2xml({ "setSysInfo": { "wifiSwitch": toAttribute(attr)}});	

	//发出请求不一定会返回，有时候可以返回有时候不能，所以发送以后直接倒计时，提示连接成功。
	$.ajax({ url: appendTS(api.apiurl),
	    type: "POST",
	    data: { data: xml },
	    contentType: "application/x-www-form-urlencoded;",
	    success: function (result) {
	    	
	        //Loaded();
	        var result = $.xml2json(result);
	        if(result.Return == undefined)
	        {
				console.log("命令不支持");
				//layer.msg("切换模式失败");
				hideWorkModeLoadingView();
				//android
	        	window.LetvJSBridge_For_Android.callLeboxChangeModel(true);
				return;
	        }

	        if ((result.Return != undefined)  && (result.Return.status == "false"))
	        {
	        	//alert(result.Return.toString());
	        	console.log("失败");
	        	//提示格式化失败

	        	//layer.msg("切换模式失败");
	        	hideWorkModeLoadingView();
	        	//android
	        	window.LetvJSBridge_For_Android.callLeboxChangeModel(true);
	        	return;

	        } 


	        var delay = result.Return.delay;
	        if(delay != undefined)
	        {
	        	setTimeout(function() {
			        hideWorkModeLoadingView();
			        console.log("命令请求成功！");
			        //format_disk_progress();
			        layer.msg("切换模式成功",{time:1000});
			        //开始刷新界面
			        initView();
			        //android
		        	window.LetvJSBridge_For_Android.callLeboxChangeModel(true);
	        	}, delay * 1000);
	        }
	        else
	        {
	        	//layer.msg("切换模式失败");
		        hideWorkModeLoadingView();
		        console.log("命令请求成功！");
		        //format_disk_progress();
		        //开始刷新界面
		        initView();
		        //android
	        	window.LetvJSBridge_For_Android.callLeboxChangeModel(true);
	        }

	    },
	    error:function(x,e)
	    {
	    	//layer.msg("切换模式失败");
	    	hideWorkModeLoadingView();
	    	//android
	        window.LetvJSBridge_For_Android.callLeboxChangeModel(true);
	    }
	});

	showWorkModeLoadingView();	
}

function click_dialog_switch_workmode()
{
	layer.closeAll();
//	alert("这里处理连接操作");
	console.log("click_dialog_switch_workmode() 这里处理连接操作");
	//todo 
	var curWorkModeText = $("#workmode").html();
	if(curWorkModeText == "路由模式")
//	if(curWorkModeText == "WiFi模式")
	{
		switch_workmode(1);
	}
	else
	{
		switch_workmode(0);
	}
	
}


function initClick()
{
　$("#format_disk").click(function(){ 
		//当前正在请求则不能点击执行

		if(!$("#loading").is(":hidden"))
		{
			return;
		}

		var dialog = $("<div/>").addClass("dialog_root");
		var dialog_title = $("<div/>").addClass("dialog_title_red").text("磁盘格式化");
		var dialog_input = $("<div/>").addClass("dialog_format_msg").text("格式化会删除您缓存的视频，是否继续？\n（格式化后随身看会自动重启）");
		var dialog_button = $("<div/>").addClass("dialog_button");
		var dialog_button_cancel = $("<div onclick=\"click_dialog_cancel()\"/>").addClass("dialog_button_cancel").text("取消");
		var dialog_button_confirm = $("<div onclick=\"click_dialog_format()\"/>").addClass("dialog_button_confirm").text("继续");
		dialog_button.append(dialog_button_cancel).append(dialog_button_confirm);
		dialog.append(dialog_title).append(dialog_input).append(dialog_button);
		// dialog_button_cancel.click(function(){
		// 	layer.closeAll();
		// });
		// dialog_button_confirm.click(function(){
		// 	layer.closeAll();
		// });
		var html = dialog.prop("outerHTML");
		layer.open({
		    type: 1,
		    title: false,
		    closeBtn: 0,
		    shadeClose: false,
		    skin: 'yourclass',
		    content: html

		});
　　}); 


　$("#switch_workmode").click(function(){ 

		//当前正在请求则不能点击执行

		if(!$("#workmode_loading").is(":hidden"))
		{

			return;
		}


		var curWorkModeText = $("#workmode").html();
		var showMessage="路由模式和室外模式切换";
		//var showMessage="WiFi模式和室外模式切换吗？";
		if(curWorkModeText == "路由模式")
		{
			//showMessage = "切换到\"室外模式\"吗？";
			showMessage = "切换模式后随身看将与手机断开连接，请重新连接随身看";
		}
		else
		{
			showMessage = "切换模式后随身看将与手机断开连接，请重新连接随身看";
			//showMessage = "切换到\"路由模式\"吗？";
			//showMessage = "切换到\"WiFi模式\"吗？";
		}

		var dialog = $("<div/>").addClass("dialog_root");
		var dialog_title = $("<div/>").addClass("dialog_title_red").text("切换模式");
		var dialog_input = $("<div/>").addClass("dialog_msg").text(showMessage);
		var dialog_button = $("<div/>").addClass("dialog_button");
		var dialog_button_cancel = $("<div onclick=\"click_dialog_cancel()\"/>").addClass("dialog_button_cancel").text("取消");
		var dialog_button_confirm = $("<div onclick=\"click_dialog_switch_workmode()\"/>").addClass("dialog_button_confirm").text("继续");
		dialog_button.append(dialog_button_cancel).append(dialog_button_confirm);
		dialog.append(dialog_title).append(dialog_input).append(dialog_button);

		var html = dialog.prop("outerHTML");
		layer.open({
		    type: 1,
		    title: false,
		    closeBtn: 0,
		    shadeClose: false,
		    skin: 'yourclass',
		    content: html

		});
　　}); 


}

function initAjax()
{
	//网络请求错误处理
	$.ajaxSetup({
                error:function(x,e){
                	if(x.status == 404)
                	{
						alert("请求地址不存在");
                	}
                	else
                	{
                		//alert("status = " + x.status  );
                		console.log("请求失败");
                	}
                    
                    return false;
                }
            });
}

function initView()
{
	$.get(appendTS(api.apiurl), { data: api.power }, function (result) {

		 var result = $.xml2json(result);

        if (result == undefined) 
        {
        	console.log("返回值为空");
        	return;
        }
        //
        if (result.Return.status != "true") {
          //  alert(result.Return["#text"] || result.Return.toString());
             console.log(result.Return["#text"] || result.Return.toString());
            return;
        }	 	

        //获取到电量，显示界面
        var power = result.Power;
        var percent = power.percent;

         console.log("电量百分比 = " +percent+"%");
 		$("#power_percent").html(percent + "%");
         

	 });

	$.get(appendTS(api.apiurl), { data: api.storage }, function (result) {

		 var result = $.xml2json(result);

        if (result == undefined) 
        {
        	console.log("返回值为空");
        	return;
        }
        //
        if (result.Return.status != "true") {
           // alert(result.Return["#text"] || result.Return.toString());
             console.log(result.Return["#text"] || result.Return.toString());
            return;
        }	 	

        //获取到容量，显示界面

        var storage = result.Storage;
        var section = storage.Section;
        var used_byte = section.used_byte;
        var total_byte = section.total_byte;


        var divisor = 1024*1024*1024;//G
	if(used_byte<41943040) used_byte=0;



 		$("#used_percent").html( ""+(used_byte/divisor).toFixed(2) + "G/" + (total_byte/divisor).toFixed(2) + "G");




  //       var used_percent = used_byte * 100 / total_byte ;

  //       if(used_percent >= 1)
  //       {
  //       	used_percent = 0 | used_percent;
  //       }
  //       else if(used_percent < 0.1)
  //       {
  //       	used_percent = 0.1;
  //       }
  //       else
  //       {
  //       	var temp = (used_percent * 10 ) %10;
  //       	temp = 0 | temp;
  //       	used_percent = temp / 10;
  //       }
  //       console.log("使用百分比 = " +used_percent+"%");
		// var used = $("#used_percent");
  //       $("#used_percent").html("已使用容量" + used_percent + "%");




	 });

	$.get(appendTS(api.apiurl), { data: api.users }, function (result) {

		 var result = $.xml2json(result);

        if (result == undefined) 
        {
        	console.log("返回值为空");
        	return;
        }
        //
        if (result.Return.status != "true") {
            //alert(result.Return["#text"] || result.Return.toString());
            //console.log(result.Return["#text"] || result.Return.toString());
            console.log("当前不支持该命令");
            return;
        }	 	

        //获取到，显示界面
         var users = result.Users;
         var count = users.count;
         if(count > 0)
         {
         	 $("#users").html(count + "人");
         }
         else
         {
         	 $("#users").html("1人");
         }
        

	 });

	$.get(appendTS(api.apiurl), { data: api.wifiswitch }, function (result) {

		 var result = $.xml2json(result);

        if (result == undefined) 
        {
        	console.log("返回值为空");
        	return;
        }
        //
        if (result.Return.status != "true") {
            //alert(result.Return["#text"] || result.Return.toString());
            //console.log(result.Return["#text"] || result.Return.toString());
            console.log("当前不支持该命令");
            return;
        }	 	

        //获取到，显示界面
        var wifiSwitch = result.wifiSwitch
         var mode = wifiSwitch.mode;

         if(mode == 0)
         {
         	 $("#workmode").html("路由模式");
         	 //$("#workmode").html("WiFi模式");
         }
         else
         {
         	 $("#workmode").html("室外模式");
         }
        

	 });
}

/************ 初始化 ***************/
$(document).ready(function(){

	// var used = 1038323483;
	// var total = 555555555456;
	// var divisor= 1025*1024*1024;

	// $("#used_percent").html( ""+(used/divisor).toFixed(2) + "G/" + (total/divisor).toFixed(2) + "G");






	initClick();

	initAjax();

	initLayer();

	initView();


      // var users = $("#users");
      //   var value = users.html();
//	$("#users").html("未知");

});
