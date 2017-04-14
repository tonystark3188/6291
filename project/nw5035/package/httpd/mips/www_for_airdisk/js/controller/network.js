
var timeOutVar;
var timeOutCheckConnectedVar;





function showLoading()
{
	$('#loading').show();
}

function hideLoading()
{
	$('#loading').hide();
}


// 获取item的rootdiv
function creatRootDiv(totalLength,index)
{
	if(index == (totalLength - 1))
	{
		return  $("<div/>").addClass("listview_item_bottom");
	}
	else{
		return $("<div/>").addClass("listview_item_middle");
	}
};
// 获取信号对应的资源图片
// function get_rssi_res(rssi,encrypt) {
//     if (encrypt != "NONE")
//         return "img/icon_wifi_lock.png";
//     else 
//         return "img/icon_wifi_unlock.png";

//  //    if (rssi > -50)
//  //        return "img/iPad_WiFi_Symbol.png";
//  //    else if (rssi > -65)
//  //        return "img/iPad_WiFi_Symbol4.png";
//  //    else if (rssi > -80)
//  //        return "img/iPad_WiFi_Symbol3.png";
//  //    else if(rssi > -90)
//  //        return "img/iPad_WiFi_Symbol2.png";
// 	// else
// 	// 	return "img/iPad_WiFi_Symbol1.png";
// }
function get_rssi_res(encrypt) {
    if (encrypt != "NONE")
        return "img/icon_wifi_lock.png";
    else 
        return "img/icon_wifi_unlock.png";

 //    if (rssi > -50)
 //        return "img/iPad_WiFi_Symbol.png";
 //    else if (rssi > -65)
 //        return "img/iPad_WiFi_Symbol4.png";
 //    else if (rssi > -80)
 //        return "img/iPad_WiFi_Symbol3.png";
 //    else if(rssi > -90)
 //        return "img/iPad_WiFi_Symbol2.png";
	// else
	// 	return "img/iPad_WiFi_Symbol1.png";
}

function getConnectedSSIDDO(remoteAP)
{
	var rootDiv = $("<div/>").addClass("connected_ssid");
	if(remoteAP != undefined)
	{
		if(remoteAP.status == 1){
			var titleDiv = $("<div id=\"remoteApName\"/>").addClass("item_title").text(remoteAP.name + "(未连接)");
			//var valueDiv = $("<div />").addClass("item_value").append($("<img class=\"item_value_img\"/>").attr('src',get_rssi_res(remoteAP.encrypt)));
			var clearDiv = div_clean = $("<div/>").addClass("clear");
			rootDiv.append(titleDiv).append(valueDiv).append(clearDiv);	
			if(mode==0)$("#switch_item").show();
			else $("#switch_item").hide();
		}
		else
		{
			var hookDiv = $("<div/>").addClass("item_img_left").append($("<img class=\"item_img_hook\"/>").attr('src','img/icon_hook.png'));
			var titleDiv = $("<div id=\"remoteApName\"/>").addClass("item_title").text(remoteAP.name);
			var valueDiv = $("<div/>").addClass("item_value").append($("<img class=\"item_value_img\"/>").attr('src',get_rssi_res(remoteAP.encrypt)));
			var clearDiv = div_clean = $("<div/>").addClass("clear");
			rootDiv.append(hookDiv).append(titleDiv).append(valueDiv).append(clearDiv);	
			$("#switch_item").hide();	
		}		
	}
	else
	{
		var titleDiv = $("<div id=\"remoteApName\"/>").addClass("item_title").text("未连接");
		var clearDiv = div_clean = $("<div/>").addClass("clear");
		rootDiv.append(titleDiv).append(clearDiv);
		$("#switch_item").show();
	}

	

	return rootDiv;
}

//清除ap列表
function cleanApList()
{
	 $("#content").empty();
	 $("#dmui_listview").hide();
}

function appendTS(url) {
    if (url.indexOf('ts=') == -1) {
        url = url + (url.indexOf('?') == -1 ? "?ts=" : "&ts=") + (new Date()).valueOf().toString();
    } else {
        url = url.replace(/&ts=.+/, "&ts=" + (new Date()).valueOf().toString())
    }
    return url;
}


function distinctAP(aplist) {
    
//   var tb = new Hashtable();
    var apname = "", ap = null,tmpap=0;
 //   var newaplist=new Hashtable();
 	var newApList = new Array();
    for (var i = 0;i < aplist.length; i++) {
        for (var j = i+1; j < aplist.length; j++) {     
            
		if((aplist[i]["rssi"]/2) < (aplist[j]["rssi"]/2))
            {
                
			    tmpap=aplist[i];
                aplist[i]=aplist[j];
                aplist[j]=tmpap;
            }
        }
    }
    

    for(var i = 0;i < aplist.length; i++)
    {
    	ap = aplist[i];
    	temp = ap;
    	for(var j = 0; j < newApList.length; j++)
    	{
    		if(ap.name == newApList[j].name)
    		{
    			temp = null;
    			break;
    		}
    	}
    	if(temp != null)
    	{
			newApList[newApList.length] = temp;
    	}
    }
	// for (var i = 0; i < aplist.length; i++) {
 //        apname = aplist[i]["name"];
 //        if (tb.contains(apname)) {
 //            //存在，比较大小
 //            ap = tb.get(apname);
 //            if (ap.rssi < aplist[i].rssi) {
 //                ap = aplist[i];
 //            }
 //        } else {

            
	// 	tb.add(aplist[i]["name"], aplist[i]);
 //        //}
 //    }
    
	return newApList;
}

function showApList(apList)
{
	console.log(apList);
	var i;
	var ap;
	var count = apList.length;


	var newapList = distinctAP(apList);
	//清空列表
	$("#content").empty();

	for(i = 0;i<newapList.length;i++)
	{
		ap = newapList[i];
		item_root_div = creatRootDiv(count,i);
		item_title = $("<div/>").addClass("item_title").text(ap.name);

		item_value = $("<div/>").addClass("item_value").append($("<img class=\"item_value_img\"/>").attr('src',get_rssi_res(ap.encrypt)));
		div_clean = $("<div/>").addClass("clear");
		item_root_div.append(item_title);
		item_root_div.append(item_value);
		// if(ap.encrypt != "NONE")
		// {
		// 	item_value_lock = $("<div/>").addClass("item_value").append($("<img/>").attr('src','img/iPad_WiFi_Lock.png'));
		// 	item_root_div.append(item_value_lock);
		// }
		item_root_div.append(div_clean);
		//item_root_div.apName = ap.name;
		//item_root_div.attr("mydata",ap);
		item_root_div.click(ap,function(event){
			var ap = event.data;
//			alert(ap.name);
			curSeletedAP = ap;
			//连接ap
			if(ap.encrypt == "NONE")
			{
				var dialog = $("<div/>").addClass("dialog_root");
				var dialog_title = $("<div/>").addClass("dialog_title").text(ap.name);
				var dialog_input = $("<div/>").addClass("dialog_msg").text("确定连接吗？");
				var dialog_button = $("<div/>").addClass("dialog_button");
				var dialog_button_cancel = $("<div onclick=\"click_dialog_cancel()\"/>").addClass("dialog_button_cancel").text("取消");
				var dialog_button_confirm = $("<div onclick=\"click_dialog_connectAP()\"/>").addClass("dialog_button_confirm").text("连接");
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
				    scrollbar:false,
				    skin: 'yourclass',
				    content: html

				});
			}
			else
			{
				var record = ap.record;
				var password;
				if ((record != undefined) && (record == 1))
				{
					password = ap.password;
				}

				var dialog = $("<div/>").addClass("dialog_root");
				var dialog_title = $("<div/>").addClass("dialog_title").text("请输入\""+ ap.name +"\"的密码");
				var dialog_input;
				if(password != undefined)
				{
					dialog_input = $("<div/>").addClass("dialog_input")
							.append($("<div id=\"dialog_pw_div\"/>").append($("<input id=\"password\" type=\"password\" name=\"password\"  placeholder=\"密码\" value=\"" + password +"\"/>").addClass("dialog_pw_input")))
							.append($("<div/>").addClass("dialog_input_visible").append($("<img id=\"eye\" data=\"invisible\" onclick=\"pswmask()\"/>").addClass("dialog_input_visible_img").attr("src","img/icon_pw_invisible.png")));

				}
				else
				{
					dialog_input = $("<div/>").addClass("dialog_input")
							.append($("<div id=\"dialog_pw_div\"/>").append($("<input id=\"password\" type=\"password\" name=\"password\"  placeholder=\"密码\"/>").addClass("dialog_pw_input")))
							.append($("<div/>").addClass("dialog_input_visible").append($("<img id=\"eye\" data=\"invisible\" onclick=\"pswmask()\"/>").addClass("dialog_input_visible_img").attr("src","img/icon_pw_invisible.png")));

				}

				var dialog_button = $("<div/>").addClass("dialog_button");
				var dialog_button_cancel = $("<div onclick=\"click_dialog_cancel()\"/>").addClass("dialog_button_cancel").text("取消");
				var dialog_button_confirm = $("<div onclick=\"click_dialog_connectAP()\"/>").addClass("dialog_button_confirm").text("连接");
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
				    scrollbar:false,
				    skin: 'yourclass',
				    content: html

				});				
			}


		});
		$("#content").append(item_root_div);
	}

	$("#dmui_listview").show();
}

function refreshCurConnectedAP(remoteAP)
{
	var div = getConnectedSSIDDO(remoteAP);
	$("#connected_ssid_dev").empty();
	$("#connected_ssid_dev").append(div);

}

function getCurConnectedAP()
{
	 $.get(appendTS(api.apiurl), { data: api.remoteap }, function (result) {

  			 var result = $.xml2json(result);

            if (result == undefined) 
            {
            	console.log("返回值为空");
            	return;
            }
            //
            if (result.Return.status != "true") {
                alert(result.Return["#text"] || result.Return.toString());
                return;
            }	 	
	 	refreshCurConnectedAP(result.RemoteAP);
	 });

}

function refreshApList()
{
	//网络请求aplist
	$.ajax({ url: appendTS(api.apiurl),
	    type: "POST",
	    data: { data: api.aplist },
	    contentType: "application/x-www-form-urlencoded;",
	    success: function (result) {
			hideLoading();	

  			console.log("aplist 数据处理");
  			//todo

  			 var result = $.xml2json(result);

            if (result == undefined) 
            {
            	console.log("返回值为空");
            	return;
            }
            //
            if (result.Return.status != "true") {
                
                return;
            }
            //
           // theAPList = data.APList;
            showApList(result.APList.AP);
               
	        
	    },
	    error:function(x,e)
	    {
	    	hideLoading();	
	    }
	});
 //  $.get(appendTS(api.apiurl), { data: api.aplist }, function (result) {
 // // 			alert("aplist 数据处理");
 //  			console.log("aplist 数据处理");
 //  			//todo

 //  			 var result = $.xml2json(result);

 //            if (result == undefined) 
 //            {
 //            	console.log("返回值为空");
 //            	return;
 //            }
 //            //
 //            if (result.Return.status != "true") {
 //                alert(result.Return["#text"] || result.Return.toString());
 //                return;
 //            }
 //            //
 //           // theAPList = data.APList;
 //            showApList(result.APList.AP);
 //            hideLoading();
 //       	     //result = $.xml2json(result);
 //   		    // bindWLAN(result);
 //   		    // Loaded();
	//      });
  showLoading();
  cleanApList();
}

function initClick()
{
	
	$("#switch_button").click(function(){
//		alert("刷新点击事件处理");

		console.log("切换点击事件处理");
		window.location.href="/info.html";
	});	



	$("#operation_refresh").click(function(){
//		alert("刷新点击事件处理");

		console.log("刷新点击事件处理");

		//console.log("测试 isDelaying() = " + isDelaying());


		if(isRefreshing())
		{
			console.log("正在刷新中，不重复发起请求！");
			return ;
		}
		//todo
		refreshApList();
	});

	$("#operation_add").click(function(){
		//alert("添加网络点击事件处理");
		console.log("添加网络点击事件处理");
		//todo
			var dialog = $("<div/>").addClass("dialog_root");
			var dialog_title = $("<div/>").addClass("dialog_title").text("添加网络");
			var dialog_input_ssid = $("<div/>").addClass("dialog_input")
						.append($("<div id=\"dialog_line_div\"/>").append($("<input id=\"ssid\" type=\"text\"  placeholder=\"SSID\"/>").addClass("dialog_pw_input")));
			
			var dialog_input = $("<div/>").addClass("dialog_input")
						.append($("<div id=\"dialog_pw_div\"/>").append($("<input id=\"password\" type=\"password\" name=\"password\" placeholder=\"密码\"/>").addClass("dialog_pw_input")))
						.append($("<div/>").addClass("dialog_input_visible").append($("<img id=\"eye\" data=\"invisible\" onclick=\"pswmask()\"/>").addClass("dialog_input_visible_img").attr("src","img/icon_pw_invisible.png")));
			
			var dialog_button = $("<div/>").addClass("dialog_button");
			var dialog_button_cancel = $("<div onclick=\"click_dialog_cancel()\"/>").addClass("dialog_button_cancel").text("取消");
			var dialog_button_confirm = $("<div onclick=\"click_dialog_add()\"/>").addClass("dialog_button_confirm").text("添加网络");
			dialog_button.append(dialog_button_cancel).append(dialog_button_confirm);
			dialog.append(dialog_title).append(dialog_input_ssid).append(dialog_input).append(dialog_button);
			var html = dialog.prop("outerHTML");
			layer.open({
			    type: 1,
			    title: false,
			    closeBtn: 0,
			    shadeClose: false,
			    scrollbar:false,
			    skin: 'yourclass',
			    content: html

			});

	});




}

// function onItemClick(ap)
// {
// 	alert(ap.name);
// }

function initAjax()
{
	//网络请求错误处理
	$.ajaxSetup({
                error:function(x,e){
                	if(x.status == 404)
                	{
						//alert("请求地址不存在");
						console.log("initAjax error  404 请求地址不存在");
						 
                	}
                	else
                	{
                		console.log("initAjax error status = " + x.status );
                		//alert("status = " + x.status  );
                	}
                    
                    return false;
                }
            });
}


function connectAP(ap)
{
	console.log("connectAP() 开始发起连接请求到 "  + ap.name);
	//var password = $("#password").val();
	console.log("name = "  + ap.name + ";password = " + ap.password);

	curConnectAP.name = ap.name;
	curConnectAP.password = ap.password;


	//ap.password = password;
	//开始请求连接ssid
	ap.name=ap.name.replace(/&/g,"&amp;");
	ap.name=ap.name.replace(/"/g,"&quot;");
	ap.name=ap.name.replace(/</g,"&lt;");
	ap.name=ap.name.replace(/>/g,"&gt;");
	if(ap.encrypt != "NONE" )
	ap.password=ap.password.replace(/(&|\"|>|<)/g,function($0,$1){return{"&":"&amp;","\"":"&quot;",">":"&gt;","<":"&lt;"}[$1];});
	var xml = json2xml({ "setSysInfo": { "JoinWireless": { "AP": toAttribute(ap)}} });	

	//发出请求不一定会返回，有时候可以返回有时候不能，所以发送以后直接倒计时，提示连接成功。
	$.ajax({ url: appendTS(api.apiurl),
	    type: "POST",
	    data: { data: xml },
	    contentType: "application/x-www-form-urlencoded;",
	    success: function (result) {

	        //Loaded();
	        var result = $.xml2json(result);
	        if (result.Return.status != "true") alert(result.Return.toString());
	        console.log("请求成功，正在连接 " + "name = "  + ap.name + ";password = " + ap.password);




	    },
	    error:function(x,e){
	    	 console.log("请求没有收到返回");


	    }
	});


	// layer.msg('连接中请等待', {
	//     time: 5000 ,
	//     shade: 0.3,
	//     shadeClose: false,
	//     scrollbar:false 
	// });

	showTimerMsg(30000,ap);
}
//是在延时中
function isDelaying()
{
	//var obj = $('#dialog_msg_text');
	if($("#dialog_msg_text").length > 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//aplist获取中
function isRefreshing()
{
	if($("#loading").is(":hidden")) 
	{ 
		return false;
	} 
	else
	{
		return true;
	}
}

function cleanLayerAndTimeout()
{
	//停止定时器
	clearTimeout(timeOutVar);
	//倒计时完成
	layer.closeAll();
	//刷新当前界面
	refreshPage();	

}

var curConnectAP = new Object();
function checkRemoteAPConnected(remoteAP)
{
	if(remoteAP != undefined)
	{
		if(remoteAP.status == 1){
			//未连接
			//return false;
		}
		else
		{
			console.log("remoteAP.name = " + remoteAP.name + " ;curConnectAP.name = " + curConnectAP.name);
			if(remoteAP.name == curConnectAP.name)
			{
				//已连接
				return true;				
			}

		}		
	}
	else
	{
		//未连接
		//return false;
	}
	return false;
}

var isCheckConnectting = false;
function checkConnect()
{
	if(!isCheckConnectting)
	{
		isCheckConnectting = true;

			$.ajax({  
		          type : "get",  
		          url : appendTS(api.apiurl),  
		          data : { data: api.remoteap },  
		          async : true,  
		          success : function(result){  
					var result = $.xml2json(result);

			        if (result == undefined) 
			        {
			        	console.log("返回值为空");
			        	return;
			        }
			        //
			        if (result.Return.status != "true") {
			            alert(result.Return["#text"] || result.Return.toString());
			            return;
			        }	 	

					console.log("checkConnect result.RemoteAP = " + result.RemoteAP);
			        //校验当前是否已经连接成功
			        var ret = checkRemoteAPConnected(result.RemoteAP);

			        console.log("checkRemoteAPConnected ret = " + ret);

			        if(ret)
			        {
			        	cleanLayerAndTimeout();
			        	//提示连接成功
			        	//todo
			        	console.log("连接成功");
			        	layer.msg('连接成功',{time:1000});
			        }

					isCheckConnectting = false;

		          }  ,
		          error : function(x,e){
		          	isCheckConnectting = false;
		          }
         		 }); 
	}


	// $.get(appendTS(api.apiurl), { data: api.remoteap }, function (result) {

	// 		if(!hasDialog())
	// 		{
	// 			return;
	// 		}


	// 		var result = $.xml2json(result);

	//         if (result == undefined) 
	//         {
	//         	console.log("返回值为空");
	//         	return;
	//         }
	//         //
	//         if (result.Return.status != "true") {
	//             alert(result.Return["#text"] || result.Return.toString());
	//             return;
	//         }	 	

	// 		console.log("checkConnect result.RemoteAP = " + result.RemoteAP);
	//         //校验当前是否已经连接成功
	//         var ret = checkRemoteAPConnected(result.RemoteAP);

	//         console.log("checkRemoteAPConnected ret = " + ret);

	//         if(ret)
	//         {
	//         	cleanLayerAndTimeout();
	//         	//提示连接成功
	//         	//todo
	//         	console.log("checkConnect 连接成功");
	//         	layer.msg('连接成功',{time:1000});
	//         }



	//  });
}

function showTimerMsg(mtime,ap)
{
	var time = mtime/1000;


	var dialog_msg_root = $("<div/>").addClass("dialog_msg_root");
	var dialog_msg_text = $("<div id=\"dialog_msg_text\"/>").addClass("dialog_msg_text").text("连接中请稍等 " + time + "S");
//	curConnectAP = ap;


	dialog_msg_root.append(dialog_msg_text);	
	var html = dialog_msg_root.prop("outerHTML");
	//连接中请骚等
	layer.open({
	    type: 1,
	    title: false,
	    closeBtn: 0,
	    shadeClose: false,
	    skin: 'yourclass',
	    content: html
	});

	function contentap_count_down()
	{
			time --;

			if((time % 3) == 0)
			{
				checkConnect();
			}

			if(time > 0)
			{
				$('#dialog_msg_text').text("连接中请稍等 " + time + "S");
				timeOutVar = setTimeout(contentap_count_down,1000);
			}
			else
			{
				//倒计时完成
				$('#dialog_msg_text').text("连接超时,请重新连接随身看!");
				//layer.closeAll();
				//todo 处理倒计时完成以后，是否刷新页面等操作处理
				console.log("contentap_count_down() 需要处理刷新操作"  );
				//layer.msg('网络已断开，请尝试刷新！',{time:2000});
				//layer.msg('网络已断开，请尝试刷新！');
				//刷新当前界面
				//refreshPage();

			}
	}


	timeOutVar = setTimeout(contentap_count_down,1000);
	

}

function refreshPage()
{

		initView();

}



function click_dialog_cancel()
{
	layer.closeAll();
}

function click_dialog_connectAP()
{
	//要在弹窗消失以前获取节点信息
	var password = $("#password").val();


//	alert("这里处理连接操作");
	console.log("click_dialog_connectAP() 这里处理连接操作"  );

	//连接当前选择的ap
	if(curSeletedAP != null)
	{
		var ap = $.extend(true, {}, curSeletedAP);
		if(ap.encrypt == "NONE")
		{
			ap.password = "";
		}
		else
		{
			
			ap.password = password;			
			if(password.length<8) return;
		}

		layer.closeAll();
		connectAP(ap);
	}
	

}



function click_dialog_add()
{
	//要在弹窗消失以前获取节点信息
	var ssid = $('#ssid').val();
	var password = $("#password").val();
	layer.closeAll();
//	alert("这里处理连接操作");
	console.log("click_dialog_add() 添加网络"  );


	//添加网络操作
	//todo 添加网路操作
	var ap = new Object();
	ap.name = ssid;
	ap.password = password;
	ap.encrypt = "";
	ap.add_network = 1;
	
	connectAP(ap);

}

function pswmask()
{
	var data = $("#eye").attr("data");
	if(data == "invisible")
	{
		console.log("invisible");
		//$("#password").attr("type","text");
		var psDiv = $('#dialog_pw_div');
		var value = $("#password").val(); 
		psDiv.html("<input id=\"password\" type=\"text\" class=\"dialog_pw_input\" placeholder=\"密码\" value=\'"+ value +"\'/>");
		$("#eye").attr("src","img/icon_pw_visible.png");
		$("#eye").attr("data","visible");
	}
	else
	{
		console.log("visible");
		//$("#password").attr("type","password");
		var psDiv = $('#dialog_pw_div');
		var value = $("#password").val(); 
		psDiv.html("<input id=\"password\" type=\"password\" class=\"dialog_pw_input\" placeholder=\"密码\" value=\'"+ value +"\'/>");
		$("#eye").attr("src","img/icon_pw_invisible.png");
		$("#eye").attr("data","invisible");
	}
}


var curSeletedAP = null;
function initApList()
{
	var count =3;
	var ap=null,curAp = null;
	var item_root_div= null;
	var item_title = null;
	var item_value = null;
	var item_value_lock = null;
	var div_clean = null;


	curAp = new adcgi_obj_ap();
	curAp.init();

	var remoteAP = new Object();
	remoteAP.status = 1;
	remoteAP.encrypt = "wap";
	remoteAP.name = "airdisk_remoteap";

	var connectedApDo = getConnectedSSIDDO(remoteAP);
//	$("#container").prepend(connectedApDo);
	$("#connected_ssid_dev").append(connectedApDo);

	for(i= 0;i<count;i++){
		ap = new adcgi_obj_ap();
		ap.init();
		if(i%2 == 0)
		{
			ap.encrypt = "NONE";
		}
		else
		{
			ap.encrypt = "WAP"
		}
		ap.name = ap.name + i;

		item_root_div = creatRootDiv(count,i);
		item_title = $("<div/>").addClass("item_title").text(ap.name);

		item_value = $("<div/>").addClass("item_value").append($("<img class=\"item_value_img\"/>").attr('src',get_rssi_res(ap.encrypt)));
		div_clean = $("<div/>").addClass("clear");
		item_root_div.append(item_title);
		item_root_div.append(item_value);
		// if(ap.encrypt != "NONE")
		// {
		// 	item_value_lock = $("<div/>").addClass("item_value").append($("<img/>").attr('src','img/iPad_WiFi_Lock.png'));
		// 	item_root_div.append(item_value_lock);
		// }
		item_root_div.append(div_clean);
		//item_root_div.apName = ap.name;
		//item_root_div.attr("mydata",ap);
		item_root_div.click(ap,function(event){
			var ap = event.data;
//			alert(ap.name);
			curSeletedAP = ap;
			//连接ap

			var dialog = $("<div/>").addClass("dialog_root");
			var dialog_title = $("<div/>").addClass("dialog_title").text("请输入\"letv-office\"的密码");
			var dialog_input = $("<div/>").addClass("dialog_input")
						.append($("<div id=\"dialog_pw_div\"/>").append($("<input id=\"password\" type=\"password\" name=\"password\"  placeholder=\"密码\"/>").addClass("dialog_pw_input")))
						.append($("<div/>").addClass("dialog_input_visible").append($("<img id=\"eye\" data=\"invisible\" onclick=\"pswmask()\"/>").addClass("dialog_input_visible_img").attr("src","img/icon_pw_invisible.png")));
			var dialog_button = $("<div/>").addClass("dialog_button");
			var dialog_button_cancel = $("<div onclick=\"click_dialog_cancel()\"/>").addClass("dialog_button_cancel").text("取消");
			var dialog_button_confirm = $("<div onclick=\"click_dialog_connectAP()\"/>").addClass("dialog_button_confirm").text("连接");
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
			    scrollbar:false,
			    skin: 'yourclass',
			    content: html

			});

		});
		$("#content").append(item_root_div);
	}	
}
function getQueryString(name) {
	var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
	var r = window.location.search.substr(1).match(reg);
	if (r != null) return unescape(r[2]); return null;
}

function initView(){

	$("#dmui_listview").hide();
	var from=getQueryString("from");
	
	if(from=="ssk") $("#switch_button").hide();
		
	var remoteAP;
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
         mode = wifiSwitch.mode;
	});
	var div = getConnectedSSIDDO(remoteAP);
	$("#connected_ssid_dev").empty();
	$("#connected_ssid_dev").append(div);

	$.get(appendTS(api.apiurl), { data: api.remoteap }, function (result) {

				 var result = $.xml2json(result);

	        if (result == undefined) 
	        {
	        	console.log("返回值为空");
	        	return;
	        }
	        //
	        if (result.Return.status != "true") {
	            alert(result.Return["#text"] || result.Return.toString());
	            return;
	        }	 	
	 	refreshCurConnectedAP(result.RemoteAP);

	 	refreshApList();
	 });
}

/************ 初始化 ***************/
$(document).ready(function(){

	initClick();

	initAjax();

	initLayer();

//	initApList();
	initView();

//	mytest();
});

function mytest()
{
	console.log("onBack = " + onBack());
	timeOutVar = setTimeout(mytest,1000);
}
