jQuery(function() {
    const TIMEOUT_REQUEST = 100;


	var mqtt = { hostname: get_appropriate_ws_url(), port: 8080 };
	var topic_configuration = "calibration/configuration";
	var topic_cmd     	= "calibration/cmd";
	var topic_cmd_response  = "calibration/cmd_response";

    var number_calibration=0;
    var filter_type=-1;
	var selectedChannel = -1;
	
	var currentCmd={};

    var requestManual = 0;
	var idxParameters = 0;
    var commands = [
        "CE"
        ,"CM 1"
        ,"CM 2"
        ,"CM 3"
        ,"CI"
        ,"MR"
        ,"DS"
        ,"DP"
        ,"CG"
        ,"ZT"
        ,"ZR"
        ,"ZI"
        ,"NT"
        ,"NR"
        ,"FM"
        ,"FL"
        ,"IV"
    ];

    var idxCurrentParameters = 0;
    var current_commands = [
        "GW",
        "GS"
    ];

    var command_CZ = [
        "CE",
        "CZ"
    ];

    var command_CG = [
        "CE",
        "CG",
    ];

    var command_CS = [
        "CE",
        "CS",
        "CE"
    ];

    var command_FM_Write = [
         "FM",
         "FL",
         "NR",
         "NT",
         "FM",
         "FL",
         "NR",
         "NT"
    ];

    var command_FM_Save = [
         "WP"
    ];

    var command_IZ = [
         "CE",
         "IZ"
    ];

    var command_CP_Write = [
        "CE",
        "DP",
        "CE",
        "DS",
        "CE",
        "MR",
        "CE",
        "CI",
        "CE",
        "CM 1",
        "CE",
        "CM 2",
        "CE",
        "CM 3",
        "CE",
        "ZI",
        "CE",
        "ZR",
        "CE",
        "ZT",
        "DP",
        "DS",
        "MR",
        "CI",
        "CM 1",
        "CM 2",
        "CM 3",
        "ZI",
        "ZR",
        "ZT"
    ];

    var execCommand;
    var disable_work = 0;
    var idxExecCommand = 0;
	
	let print=1;
	
	function print_log(msg) {
		if (print) {
			console.log (msg);
		}
	}
	
	function get_appropriate_ws_url() {
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
	
	function initialize( first) {

		selectedChannel = -1;
		filter_type = -1;

        if( first) {
		    $("#device > option").each(function() {
				$(this).remove();
		        });
		    $("#device").append($("<option>", { value: -1, text: "-1: канал не выбран"}));
		    $("#l_device").css("color","#ddd");
        }

        $("#Net").val("0.000");
        $("#Brutto").val("0.000");
        $("#val_GS").val("00000");
        $("#isMotion").css("background-color", "#f00");

		$("#filter_mode").val(0);
		$("#filter_mode").prop("disabled", true);
		$("#filter_value > option").each(function() {
			$(this).remove();
		});
		$("#filter_value").prop("disabled", true);
		$("#val_NR input").val(0);
        $("#val_NR input").prop("disabled", true);
		$("#val_NT input").val(0);
        $("#val_NT input").prop("disabled", true);
		$("#FM_Write").addClass("button_disabled");
		$("#FM_Save").addClass("button_disabled");
		
		$("#val_AZ input").val(0);
		$("#val_AZ input").prop("disabled", true);
		$("#val_CI").val(0);
		$("#val_CI input").prop("disabled", true);
		$("#val_AG input").val(0);
		$("#val_AG input").prop("disabled", true);
		$("#val_CM1 input").val(0);
		$("#val_CM1 input").prop("disabled", true);
		$("#val_AGD input").val(0);
		$("#val_AGD input").prop("disabled", true);
		$("#val_CM2 input").val(0);
		$("#val_CM2 input").prop("disabled", true);
		$("#decimal_point").val(0);
		$("#decimal_point").prop("disabled", true);
		$("#val_CM3 input").val(0);
		$("#val_CM3 input").prop("disabled", true);
		$("#display_step").val(1);
		$("#display_step").prop("disabled", true);
		$("#val_ZI input").val(0);
		$("#val_ZI input").prop("disabled", true);
		$("#multi_range").val(0);
		$("#multi_range").prop("disabled", true);
		$("#val_ZR input").val(0);
		$("#val_ZR input").prop("disabled", true);
		$("#firmware").val(0);
		$("#firmware").prop("disabled", true);
		$("#val_ZT input").val(0);
		$("#val_ZT input").prop("disabled", true);
		$("#b_CP_Write").addClass("button_disabled");
		$("#cnt_calibration input").val(0);
		$("#b_SaveCalibration").addClass("button_disabled");
		
		$("#val_CG input").val(0);
		$("#val_CG input").prop("disabled", true);
		$("#b_CZ").addClass("button_disabled");
		$("#min_span").val(1);
		$("#min_span").prop("disabled", true);
		$("#b_CG").addClass("button_disabled");
		$("#info_calib").val("");
		$("#b_IZ").addClass("button_disabled");
		
		$('#text_response').val("");
		$("#b_Clear").addClass("button_disabled");
		$("#i_CommandRequest").val("");
		$("#i_CommandRequest").prop("disabled", true);
		$("#b_Send").addClass("button_disabled");
		
	}

	function enable_interface() {
		$("#filter_mode").prop("disabled", false);
		$("#filter_value").prop("disabled", false);
        $("#val_NR input").prop("disabled", false);
        $("#val_NT input").prop("disabled", false);
		$("#FM_Write").removeClass("button_disabled");
		$("#FM_Save").removeClass("button_disabled");

		$("#val_AZ input").prop("disabled", false);
		$("#val_CI input").prop("disabled", false);
		$("#val_AG input").prop("disabled", false);
		$("#val_CM1 input").prop("disabled", false);
		$("#val_AGD input").prop("disabled", false);
		$("#val_CM2 input").prop("disabled", false);
		$("#decimal_point").prop("disabled", false);
		$("#val_CM3 input").prop("disabled", false);
		$("#display_step").prop("disabled", false);
		$("#val_ZI input").prop("disabled", false);
		$("#multi_range").prop("disabled", false);
		$("#val_ZR input").prop("disabled", false);
		$("#firmware").prop("disabled", false);
		$("#val_ZT input").prop("disabled", false);
		$("#b_CP_Write").removeClass("button_disabled");
		$("#b_SaveCalibration").removeClass("button_disabled");

		$("#val_CG input").prop("disabled", false);
		$("#b_CZ").removeClass("button_disabled");
		$("#min_span").prop("disabled", false);
		$("#b_CG").removeClass("button_disabled");
		$("#b_IZ").removeClass("button_disabled");

		$("#b_Clear").removeClass("button_disabled");
		$("#i_CommandRequest").prop("disabled", false);
		$("#b_Send").removeClass("button_disabled");
	}
	
	
// called when the client loses its connection
	function onConnectionLost(responseObject) {
		if (responseObject.errorCode !== 0) {
			print_log("onConnectionLost: "+responseObject.errorMessage);
		}
//		initialize(0);
	}

	function on_calibration_configuration( message) {
			
		$("#device > option").each(function() {
			$(this).remove();
		});
		$("#device").append($("<option>", { value: -1, text: "-1: канал не выбран"}));
			
//		print_log( message.payloadString.length);
		var parse_payload = JSON.parse( message.payloadString);
		
//		print_log( parse_payload.length);
		for( var i=0; i< parse_payload.length; ++i) {
			var dev_str = parse_payload[i].id + " канал: " 
				+ parse_payload[i].model
				+ ", " + parse_payload[i].dev 
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
	}

    function  parse_packet( packet) {
		var rc = 0;
        if( packet.cmd == "CE\r") {
            if( packet.response[0] == 'E') {
                var value = packet.response.substr(1);
                number_calibration  = parseInt(value, 10);
                $('#cnt_calibration input').val(number_calibration.toString());
				rc = 1;
            }
			else {
				print_log( "ERROR: " + packet);
			}
		}
        else if( packet.cmd == "GS\r") {
            if( packet.response[0] == 'S') {
                var value = packet.response.substr(1);
                if(  (packet.response[1] == '+') ||  (packet.response[1] == '-')) {
                     $('#val_GS').val(parseInt(value, 10).toString());
                }
                else {
                     if( packet.response[1] = 'u')
                         $('#val_GS').val( "недогруз");
                     else if( packet.response[1] = 's')
                         $('#val_GS').val( "перегруз");
                     else
                         $('#val_GS').val( value);
                }

				rc = 1;
			}
			else
				print_log( "ERROR: " + packet);
        }
        else if( packet.cmd == "GW\r") {
			if( packet.response[0] == 'W') {
                var netto = packet.response.substr(1, 6);
                if( (netto[0] == '+') || (netto[0] == '-')) {
                    $('#Net').val(parseFloat(netto).toString());
                }
                else if( netto[0] == 'u')
                    $('#Net').val("недогруз");
                else if( netto[0] == 's')
                    $('#Net').val("перегруз");
                else
                    $('#Net').val(netto);

                var brutto = packet.response.substr(7, 6);
                if( (brutto[0] == '+') || (brutto[0] == '-')) {
                    $('#Brutto').val(parseFloat(brutto).toString());
                }
                else if( brutto[0] == 'u')
                    $('#Brutto').val("недогруз");
                else if( netto[0] == 's')
                    $('#Brutto').val("перегруз");
                else
                    $('#Brutto').val(brutto);

                var status = packet.response[14];
//parseFloat("554,20".replace(",", "."));
                if( !(status & 1)) {
                    $('#isMotion').css("background-color", "#f00");
                }
                else {
                    $('#isMotion').css("background-color", "#0f0");
                }
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
        }
        else if( packet.cmd == "CM 1\r") {
			if( packet.response[0] == 'M') {
                var value = packet.response.substr(1);
                $('#val_CM1 input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "CM 2\r") {
			if( packet.response[0] == 'M') {
                var value = packet.response.substr(1);
                $('#val_CM2 input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "CM 3\r") {
			if( packet.response[0] == 'M') {
                var value = packet.response.substr(1);
                $('#val_CM3 input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "CI\r") {
			if( packet.response[0] == 'I') {
                var value = packet.response.substr(1);
                $('#val_CI input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "MR\r") {
			if( packet.response[0] == 'M') {
                var value = parseInt(packet.response.substr(1));
                $('#multi_range').val( value);
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "DS\r") {
			if( packet.response[0] == 'S') {
                var value = parseInt(packet.response.substr(1));
                $('#display_step').val( value);
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "DP\r") {
			if( packet.response[0] == 'P') {
                var value = parseInt(packet.response.substr(1));
                $('#decimal_point').val( value);
				rc = 1;
			}
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "CG\r") {
			if( packet.response[0] == 'G') {
                var value = packet.response.substr(1);
                $('#val_CG input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "ZT\r") {
			if( packet.response[0] == 'Z') {
                var value = packet.response.substr(2);
                $('#val_ZT input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "ZR\r") {
			if( packet.response[0] == 'R') {
                var value = packet.response.substr(1);
                $('#val_ZR input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "ZI\r") {
			if( packet.response[0] == 'R') {
                var value = packet.response.substr(1);
                $('#val_ZI input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "NT\r") {
			if( packet.response[0] == 'T') {
                var value = packet.response.substr(1);
                $('#val_NT input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "NR\r") {
			if( packet.response[0] == 'R') {
                var value = packet.response.substr(1);
                $('#val_NR input').val(parseInt(value, 10).toString());
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "FM\r") {
			if( packet.response[0] == 'M') {
                filter_type = parseInt(packet.response.substr(1));
                $('#filter_mode').val( filter_type);
                setFilterMode( filter_type);
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
		}
        else if( packet.cmd == "FL\r") {
			if( packet.response[0] == 'F') {
                var value = packet.response.substr(1);
                $('#filter_value').val(parseInt(value, 10));
				rc = 1;
            }
			else
				print_log( "ERROR: " + packet);
	}
        else if( packet.cmd == "IV\r") {
		if( packet.response[0] == 'V') {
                    var value = packet.response.substr(2);
                    $('#po_version input').val(value);
		    rc = 1;
                }
		else {
		    print_log( "ERROR: " + packet);
		}
	}		
	return rc;

	}

	function on_calibration_cmd_response( message)
	{
//		print_log( message.payloadString.length);
		var parse_payload = JSON.parse( message.payloadString);
		if( parse_payload) {
// выполниние списка команд	по нажатию кнопки
            if( currentCmd.mode == 'E') {
            print_log( parse_payload);
                if( parse_payload.ret_code == "OK") {
                    parse_packet( parse_payload);

                    idxExecCommand++;
                    setTimeout( execCommand, TIMEOUT_REQUEST); // milliseconds
                }
                else {
                    exec_command = "";
                    disable_work = 0;
                }
				currentCmd = {};
            }
// команда посланная вручную			
			else if( currentCmd.mode == 'M') {
				if( parse_payload.ret_code == "OK") {
					$('#text_response').val($.trim($('#text_response').val() + '\n' + parse_payload.response ));
                                        parse_packet( parse_payload);
                    parse_packet( parse_payload);
				}
				else  {
					$('#text_response').val($.trim($('#text_response').val() + '\n' 
						+ parse_payload.ret_code + " : " + parse_payload.response));
				}
				currentCmd = {};
                setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds
			}
// выполнение списка команд	запроса текущих парметров		
            else if( currentCmd.mode == 'C') {
                if( parse_payload.ret_code == "OK") {
                    parse_packet( parse_payload);
                }
                currentCmd = {};
                idxCurrentParameters++;
   	            setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds
            }
// выполнение начального списка команд (вычитывание пераметров LDU)			
            else if( currentCmd.mode == 'A') {
				if( parse_payload.ret_code == "OK") {
                    parse_packet( parse_payload);
                }
                currentCmd = {};
                idxParameters++;
// есть еще запрашиваемые параметры					
                if( idxParameters < commands.length) {
					setTimeout( getParameters, TIMEOUT_REQUEST); // milliseconds
                }
// если нет, начнем запрашивать текущие парметры					
                else {
					enable_interface();
                    idxCurrentParameters=0;
			        setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds
                }
			}
		}
		
//		print_log( parse_payload);
	}
	
// called when a message arrives
	function onMessageArrived(message) {
		print_log("onMessageArrived: "+ message.topic + " = "  + message.payloadString);
		if( message.topic == topic_configuration) {
			on_calibration_configuration( message);
			currentCmd = {};
		}
		else if( message.topic == "calibration/cmd_response") {
			on_calibration_cmd_response( message);
		}
	}

// called when the client connects
	function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
		print_log("onConnect");
		client.subscribe(topic_configuration);
		client.subscribe(topic_cmd_response);

	}

// Create a client instance
	var client = new Paho.Client(mqtt.hostname, Number(mqtt.port), "calibration-client-id");

// set callback handlers
	client.onConnectionLost = onConnectionLost;
	client.onMessageArrived = onMessageArrived;

// connect the client
	print_log("attempting to connect...")
	client.connect({onSuccess:onConnect, useSSL: false, userName:"matrix", password:"1234"});	
	
	
	$('#b_Clear').on( "click", function() {
		$('#text_response').val('');	
	});

    function setFilterMode( mode) {
		$("#filter_value > option").each(function() {
			$(this).remove();
		});
		if( mode == 0) {
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
		else if( mode == 1) {
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

        }

	$("#filter_mode").on('change', function() {
        var selectedValue = $(this).val();
        setFilterMode( selectedValue);

    });

// 
	function getParameters() {
//print_log( "" + selectedChannel + " " + Object.keys(currentCmd).length + " " + idxParameters);
		if( selectedChannel == -1)
			return;

// если нет текущей выполняемой команды		
        if (Object.keys(currentCmd).length == 0) {
// есть еще команды на выполнение ?			
            if( idxParameters <  commands.length) {
				var json = { "chan": selectedChannel , "cmd": commands[idxParameters] + '\r'}
				var str_json = JSON.stringify(json);
				client.publish(topic_cmd, str_json, 1, false);
				currentCmd = json;
				currentCmd.mode = 'A';
            }
        }
// иначе, через таймаут проверим возможность посылки следующей команды		
        else
   			setTimeout( getParameters, TIMEOUT_REQUEST); // milliseconds

	}

	function getCurrentParameters() {
		if( selectedChannel == -1)
			return;

                if( disable_work) {
                 setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds
                    return;
                }

                if( requestManual) {
                 setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds
 //                   sleep( 1);
                }
                if (Object.keys(currentCmd).length == 0) {
                       idxCurrentParameters %= current_commands.length;
		       var json = { "chan": selectedChannel, "cmd": current_commands[idxCurrentParameters] + '\r'}
		       var str_json = JSON.stringify(json);
		       client.publish(topic_cmd, str_json, 1, false);
		       currentCmd = json;
		       currentCmd.mode = 'C';
                }
                else
                	setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds

	}

	$("#device").on('change', function() {
        selectedChannel = $(this).val();
		print_log("selectedChannel: " + selectedChannel);
		if( selectedChannel == -1) {
			var json = { "chan": "-1" , "cmd": ""}
			var str_json = JSON.stringify(json);
			client.publish(topic_cmd, str_json, 1, false);
            initialize(0);
		}
		else {
// Запуск последовательного опроса LDU
			idxParameters = 0;
			setTimeout( getParameters, TIMEOUT_REQUEST); // milliseconds
		}
	});	

        function sendManual() {

		if( selectedChannel == -1)
			return;

                if(  Object.keys(currentCmd).length != 0) {
                     setTimeout( sendManual, 10);
                     requestManual = 1;
                     return;
                }

               requestManual = 0;

		var cmd = $("#i_CommandRequest").val().toUpperCase();
		print_log(cmd);
		var json = { "chan": selectedChannel , "cmd": cmd + '\r'}
		var str_json = JSON.stringify(json);
		client.publish(topic_cmd, str_json, 1, false);
		currentCmd = json;
		currentCmd.mode = 'M';
        }

	$("#b_Send").on('click', sendManual);

        function execCommand()
        {
        print_log( exec_command.length);
 //       print = 0;
                 if( idxExecCommand < exec_command.length) {
                     if(  Object.keys(currentCmd).length != 0) {
                          setTimeout( execCommand, 10);
                          return;
                     }
		     var json = { "chan": selectedChannel , "cmd": exec_command[idxExecCommand] + '\r'}
		     var str_json = JSON.stringify(json);
		     client.publish(topic_cmd, str_json, 1, false);
	      	     currentCmd = json;
		     currentCmd.mode = 'E';
                 }
                 else {
                      exec_command = "";
                      disable_work = 0;
                 }
        }

        $("#b_CZ").on('click', function() {
          if( selectedChannel == -1)
              return;
          exec_command = command_CZ.slice();
          exec_command[0] = exec_command[0] + " " + number_calibration;
          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);
	});

        $("#b_CG").on('click', function() {
          if( selectedChannel == -1)
              return;
          exec_command = command_CG.slice();
          exec_command[0] = exec_command[0] + " " + number_calibration;
          exec_command[1] = exec_command[1] + " " + $('#val_CG input').val();
          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);
	});

          $("#b_SaveCalibration").on('click', function() {
          if( selectedChannel == -1)
              return;
          exec_command = command_CS.slice();
          exec_command[0] = exec_command[0] + " " + number_calibration;
          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);
	});

        $("#FM_Write").on('click', function() {
          if( selectedChannel == -1)
              return;

          exec_command = command_FM_Write.slice();
          var f_type = $("#filter_mode").val();
          var f_value = $("#filter_value").val();
          var val_NR = $("#val_NR input").val();
          var val_NT = $("#val_NT input").val();

          exec_command[0] = exec_command[0] + " " +  f_type;
          exec_command[1] = exec_command[1] + " " +  f_value;
          exec_command[2] = exec_command[2] + " " +  val_NR;
          exec_command[3] = exec_command[3] + " " +  val_NT;

          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);

	});

        $("#FM_Save").on('click', function() {
          if( selectedChannel == -1)
              return;

          exec_command = command_FM_Save.slice();

          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);

	});

        $("#b_IZ").on('click', function() {
          if( selectedChannel == -1)
              return;

          exec_command = command_IZ.slice();
          exec_command[0] = exec_command[0] + " " + number_calibration;

          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);

	});

        $("#b_CP_Write").on('click', function() {
          if( selectedChannel == -1)
              return;

          var val_DP = $("#decimal_point").val();
          var val_DS = $("#display_step").val();
          var val_MR = $("#multi_range").val();
          var val_CI = $("#val_CI input").val();
          var val_CM1 = $("#val_CM1 input").val();
          var val_CM2 = $("#val_CM2 input").val();
          var val_CM3 = $("#val_CM3 input").val();
          var val_ZI = $("#val_ZI input").val();
          var val_ZR = $("#val_ZR input").val();
          var val_ZT = $("#val_ZT input").val();

          exec_command = command_CP_Write.slice();
          exec_command[0] = exec_command[0] + " " + number_calibration;
          exec_command[1] = exec_command[1] + " " + val_DP;
          exec_command[2] = exec_command[2] + " " + number_calibration;
          exec_command[3] = exec_command[3] + " " + val_DS;
          exec_command[4] = exec_command[4] + " " + number_calibration;
          exec_command[5] = exec_command[5] + " " + val_MR;
          exec_command[6] = exec_command[6] + " " + number_calibration;
          exec_command[7] = exec_command[7] + " " + val_CI;
          exec_command[8] = exec_command[8] + " " + number_calibration;
          exec_command[9] = exec_command[9] + " " + val_CM1;
          exec_command[10] = exec_command[10] + " " + number_calibration;
          exec_command[11] = exec_command[11] + " " + val_CM2;
          exec_command[12] = exec_command[12] + " " + number_calibration;
          exec_command[13] = exec_command[13] + " " + val_CM3;
          exec_command[14] = exec_command[14] + " " + number_calibration;
          exec_command[15] = exec_command[15] + " " + val_ZI;
          exec_command[16] = exec_command[16] + " " + number_calibration;
          exec_command[17] = exec_command[17] + " " + val_ZR;
          exec_command[18] = exec_command[18] + " " + number_calibration;
          exec_command[19] = exec_command[19] + " " + val_ZT;

          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);

	});

        function f_password() {
                 $("#myModalPasswd").css("display", "block");
	         $("#input_pswd").val("");
	         $("#chk_pswd").html("");
        }

        $("#BtnEnter").on('click', function() {
            var input = $("#input_pswd").val();
            if(  input == "12345678")
                 $("#myModalPasswd").css("display", "none");
            else
		$("#chk_pswd").html("Неверный пароль");
        });

        $("#BtnExit").on('click', function() {
//                $("#myModalPasswd").css("display", "none");
                window.close();
        });

	initialize(1);
        f_password();

	
//	enable_interface();
});
