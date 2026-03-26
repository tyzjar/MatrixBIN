jQuery(function() {
//	var mqtt = { hostname: "192.168.56.101", port: 8080 };
	var mqtt = { hostname: get_appropriate_ws_url(), port: 8080 };
	var topic_configuration = "matrix/configuration";
	var topic_status = "matrix/status";
	var topic_wgt = "matrix/wgt";
	var topic_cmd     = "matrix/cmd";
	var topic_cmd_response     = "matrix/cmd_response";

	var channel = 0;

	var currentWeightMode = ' ';
	var Configuration = {};
	var StaticMode = {};
	StaticMode.currentRange = -1;
	
	var ViewModeX10 = 0;
	const timeWorkViewModeX10 = 0; // 10 * 1000;	// milliseconds

	let print=1;
	
function print_log(msg) {
    if (print) {
		console.log (msg);
	}
}	
function get_appropriate_ws_url()
{
	var u = document.URL;

	/*
	 * We open the websocket encrypted if this page came on an
	 * https:// url itself, otherwise unencrypted
	 */

	if( u.substring(0, 4) == "file")
		return "192.168.56.101";
	
	if (u.substring(0, 5) == "https") {
		u = u.substr(8);
	} else {
		if (u.substring(0, 4) == "http")
			u = u.substr(7);
	}

	u = u.split('/');

	return u[0];
}	


function getCurDateTime()
{
	var now     = new Date(); 
    var year    = now.getFullYear();
    var month   = now.getMonth()+1; 
    var day     = now.getDate();
    var hour    = now.getHours();
    var minute  = now.getMinutes();
    var second  = now.getSeconds(); 
    if(month.toString().length == 1) {
 	   month = '0'+month;
    }
    if(day.toString().length == 1) {
       day = '0'+day;
    }   
    if(hour.toString().length == 1) {
       hour = '0'+hour;
    }
    if(minute.toString().length == 1) {
       minute = '0'+minute;
    }
    if(second.toString().length == 1) {
       second = '0'+second;
    }   
    var dateTime = day + '/' +month +'/' + year + ' ' + hour + ':' + minute + ':' + second;   
    return dateTime;
}

// called when the client loses its connection
	function onConnectionLost(responseObject) {
		if (responseObject.errorCode !== 0) {
			print_log("onConnectionLost: "+responseObject.errorCode);
		}
		$("#device > option").each(function() {
				$(this).remove();
		});
		$("#device").append($("<option>", { value: 0, text: "0: канал не выбран"}));
		$("#l_device").css("color","#ddd");
		
		
	}

	function on_matrix_configuration( message) {
/*			
		$("#device > option").each(function() {
			$(this).remove();
		});
*/			
//		print_log( message.payloadString.length);
		Configuration = JSON.parse( message.payloadString);
		if( Configuration) {
//		print_log( parse_payload);
//			Configuration = parse_payload;
			print_log( Configuration);
			addMsg( "Кофигурация получена");
			client.subscribe( topic_status);
			client.subscribe( topic_wgt);

			$("#id_stability").css("visibility", "hidden"); // visible
			$("#id_weight").text( "".padStart( 6, " "));
		}
		else 
			addMsg( "ОШИБКА: неверный пакет конфигурации");
/*		
		for( var i=0; i< parse_payload.length; ++i) {
			var dev_str = parse_payload[i].id + " канал: " 
				+ parse_payload[i].dev 
				+ ", " + parse_payload[i].tty 
				+ ", " + parse_payload[i].baud 
				+ ", " + parse_payload[i].data
				+ ", " + parse_payload[i].stop
				+ ", " + parse_payload[i].flow
				+ "," + parse_payload[i].parity;
			print_log( dev_str);
			$("#device").append($("<option>", { value: parse_payload[i].id, text: dev_str}));
		}
		if( parse_payload.length > 0) {
			$("#l_device").css("color","#222");
		}
		else {
			$("#l_device").css("color","#ddd");
		}
*/	
	}

	function on_matrix_cmd_response( message)
	{
		print_log( message.payloadString.length);
		var parse_payload = JSON.parse( message.payloadString);
		print_log( parse_payload);
	}

	function changeUnit( unit)
	{
		$("#id_range_max span").last("span").text(unit);
		$("#id_range_min span").last("span").text(unit);
		$("#id_range_discreteness span").last("span").text(unit);
		$("#id_weight_unit span").text(unit);
	}
	
	function on_matrix_status( message)
	{
		var parse_payload = JSON.parse( message.payloadString);
		print_log( parse_payload);

		if( parse_payload.WeightMode != currentWeightMode) {
			currentWeightMode = parse_payload.WeightMode;
			if( currentWeightMode == 'S') {
				changeUnit( Configuration.static_mode.unit);
			}
		}
	}

	function setRangeStatic( Range)
	{
		var num  = Number(Range);
		StaticMode.currentRange = Range;
		$("#id_range_number span").text((num + 1).toString());
//		print_log( Configuration);
		$("#id_range_max span").first("span").text(Configuration.static_mode.ranges[num].max.padStart( 6, " "));
		$("#id_range_min span").first("span").text(Configuration.static_mode.ranges[num].min.padStart( 6, " "));
		$("#id_range_discreteness span").first("span").text(Configuration.static_mode.ranges[num].step.padStart( 3, " "));		
		
	}

	function on_matrix_wgt( message)
	{
//		print_log( message.payloadString.length);
		var parse_payload = JSON.parse( message.payloadString);
		print_log( parse_payload);
		if( parse_payload) {
			if( typeof(parse_payload.Range) == "undefined")
				return;
			
//			print_log( Configuration);
			
			if( currentWeightMode == 'S') {
				if( StaticMode.currentRange != parse_payload.Range) {
					setRangeStatic( parse_payload.Range); 
				}


				if( parse_payload.State == '2') {
					$("#id_weight").first("span").text( "недогр".padStart( 6, " "));
					$("#id_stability").css("visibility", "hidden"); 
					$("#b_set_zero").addClass("button_disabled");
					$("#b_tare").addClass("button_disabled");
					return;
				}
				if( parse_payload.State == '3') {
					$("#id_weight").first("span").text( "перегр".padStart( 6, " "));
					$("#id_stability").css("visibility", "hidden"); 
					$("#b_set_zero").addClass("button_disabled");
					$("#b_tare").addClass("button_disabled");
					return;
				}

				if( parse_payload.State == '1') {
					$("#id_stability").css("visibility", "visible");
					$("#b_set_zero").addClass("button_disabled");
					$("#b_tare").addClass("button_disabled");
				}
				else {
					$("#id_stability").css("visibility", "hidden"); 
					$("#b_set_zero").removeClass("button_disabled");
					$("#b_tare").removeClass("button_disabled");
				}
				
				if( ViewModeX10==0) {
					if( parse_payload.SumWgtR) {
						$("#id_weight").first("span").text( parse_payload.SumWgtR.padStart( 6, " "));
					}
				}
				else {
//					if( parse_payload.SumWgt) {
//						$("#id_weight").first("span").text( parse_payload.SumWgt.padStart( 6, " "));
//					}
                    if( parse_payload.WgtFloatX10) {
						$("#id_weight").first("span").text( parse_payload.WgtFloatX10.padStart( 6, " "));
                    }
				}
			}
		}
	}

	
// called when a message arrives
	function onMessageArrived(message) {
//		print_log("onMessageArrived: "+ message.topic + " = "  + message.payloadString);
		if( message.topic == topic_configuration) {
			on_matrix_configuration( message);
		}
		else if( message.topic == topic_status) {
			on_matrix_status( message);
		}
		else if( message.topic == topic_wgt) {
			on_matrix_wgt( message);
		}
		else if( message.topic == topic_cmd_response) {
			print_log("onMessageArrived: "+ message.topic + " = "  + message.payloadString);
			
		}
	}

	function addMsg( msg) {
		$("#id_msg_list").append($("<li>").text( getCurDateTime() + " : " + msg));		
	}		

// called when the client connects
	function onConnect() {
		addMsg( "Соединение ОК");
//		$("#id_msg_list").append($("<li>").text( getCurDateTime() + ": Соединение ОК"));		
//		$("#id_msg_list").append($("<li>").text("Соединение ОК"));		
//		$("#id_msg_list").append($("<li>").text("Соединение ОК"));		
  // Once a connection has been made, make a subscription and send a message.
		print_log("onConnect");
		client.subscribe(topic_configuration);
		client.subscribe(topic_cmd_response);

		selectedValue = -1;
//		var json = { "cmd": "channel" , "parameter": "" + selectedValue}
//		var str_json = JSON.stringify(json);
//		client.publish(topic_cmd, str_json, 1, false)
	}

// Create a client instance
	var client = new Paho.Client(mqtt.hostname, Number(mqtt.port), "matrix-client-id");

// set callback handlers
	client.onConnectionLost = onConnectionLost;
	client.onMessageArrived = onMessageArrived;

// connect the client
	print_log("attempting to connect...")
	client.connect({onSuccess:onConnect, useSSL: false, userName:"matrix", password:"1234" });	
	
	
//	$("#Net").val("111.1")
	
// Добавить текст в начало.
//$('#text_response').val($.trim('Текст в начале.' + '\n' + $('#text_response').val()));
 
// Добавить текст в конец.
//$('#text_response').val($.trim($('#text_response').val() + '\n' + 'Текст в конце.'));

/*	
for( var i=0; i<30; ++i)
// Добавить текст в конец.
	$('#text_response').val($.trim($('#text_response').val() + '\n' + i + 'Текст в конце.' ));
*/
	$('#b_Clear').on( "click", function() {
		$('#text_response').val('');	
	});

	$("#filter_mode").on('change', function() {
        var selectedValue = $(this).val();
		$("#filter_value > option").each(function() {
			$(this).remove();
		});
		if( selectedValue == 0) {
			var filter_mode_0 = [
				{ "value" : 1, "text" : "1: 18Hz"},
				{ "value" : 2, "text" : "2: 8Hz"},
				{ "value" : 3, "text" : "3: 4Hz"},
				{ "value" : 4, "text" : "4: 3Hz"},
				{ "value" : 5, "text" : "5: 2Hz"},
				{ "value" : 6, "text" : "6: 1Hz"},
				{ "value" : 7, "text" : "7: 0.5Hz"},
				{ "value" : 8, "text" : "8: 0.25Hz"}
			];
			for( var i = 0; i < filter_mode_0.length; i++ )
				$("#filter_value").append($("<option>", { value: filter_mode_0[i].value, 
					text: filter_mode_0[i].text}));
		}
		else if( selectedValue == 1) {
			var filter_mode_1 = [
				{ "value" : 1, "text" : "1: 19.7Hz"},
				{ "value" : 2, "text" : "2: 9.8Hz"},
				{ "value" : 3, "text" : "3: 6.5Hz"},
				{ "value" : 4, "text" : "4: 4.9Hz"},
				{ "value" : 5, "text" : "5: 3.9Hz"},
				{ "value" : 6, "text" : "6: 3.2Hz"},
				{ "value" : 7, "text" : "7: 2.8Hz"},
				{ "value" : 8, "text" : "8: 2.5Hz"}
			];
			for( var i = 0; i < filter_mode_1.length; i++ )
				$("#filter_value").append($("<option>", { value: filter_mode_1[i].value, 
					text: filter_mode_1[i].text}));
		}
		
    });

	$("#device").on('change', function() {
        var selectedValue = $(this).val();
		var json = { "cmd": "channel" , "parameter": "" + selectedValue}
		var str_json = JSON.stringify(json);
		client.publish(topic_cmd, str_json, 1, false)
	});	
	
	$("#b_clean").on("click", function() {
		$("#id_msg_list").empty();
	});

	function chkViewModeX10() {
		if( ViewModeX10 == 1) {
			ViewModeX10 = 0;
            $("#b_x10").css("background", ViewModeX10 ? "linear-gradient(to top, #0EAFD4, #F1F5FB)" : "linear-gradient(to top, #9EAFD4, #F1F5FB)");
        }
	}

	$("#b_x10").on("click", function() {
		ViewModeX10 = ViewModeX10 ? 0 : 1;
		if( (ViewModeX10 == 1) && (timeWorkViewModeX10 > 0))
			setTimeout( chkViewModeX10, timeWorkViewModeX10); // milliseconds
         $("#b_x10").css("background", ViewModeX10 ? "linear-gradient(to top, #0EAFD4, #F1F5FB)" : "linear-gradient(to top, #9EAFD4, #F1F5FB)");
	});

	$("#b_set_zero").on("click", function() {
		var json = { "cmd": "SZ"}
		var str_json = JSON.stringify(json);
		client.publish(topic_cmd, str_json, 1, false)
		addMsg( "Послана команда SZ");
	});
	
        function f_password()  {
                 print_log( "Password...");
//	         $("#myModalPasswd").style.display = 'block';
                 $("#myModalPasswd").css("display", "block"); // hidden
	         $("#input_pswd").value = '';
	         $("#chk_pswd").innerHTML = ' ';
        }
//	$("#id_range_min span").text("0".padStart( 6, " "));

        f_password();
});
