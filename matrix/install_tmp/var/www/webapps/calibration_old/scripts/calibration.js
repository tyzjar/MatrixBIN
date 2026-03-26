jQuery(function() {
//	var mqtt = { hostname: "192.168.56.101", port: 8080 };
	var mqtt = { hostname: get_appropriate_ws_url(), port: 8080 };
	var topic_configuration = "calibration/configuration";
	var topic_cmd     = "calibration/cmd";
	var topic_cmd_response   = "calibration/cmd_response";

        var number_calibration=0;
        var filter_type=0;

        const TIMEOUT_REQUEST = 90;
	var selectedChan = -1;
	var currentCmd={};

        var requestManual = 0;
	var idxParameters = 0;
        var commands = [
            "CE",
            "CM 1",
            "CM 2",
            "CM 3",
            "CI",
            "MR",
            "DS",
            "DP",
            "CG",
            "ZT",
            "ZR",
            "ZI",
            "NT",
            "NR",
            "FM",
            "FL"
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

         var execCommand;
         var disable_work = 0;
         var idxExecCommand = 0;
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
	
// called when the client loses its connection
	function onConnectionLost(responseObject) {
		if (responseObject.errorCode !== 0) {
			print_log("onConnectionLost: "+responseObject.errorMessage);
		}
		$("#device > option").each(function() {
				$(this).remove();
		});
		$("#device").append($("<option>", { value: 0, text: "0: канал не выбран"}));
		$("#l_device").css("color","#ddd");
		
		
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
		selectedChanchannel = -1;
	}

        function  parse_packet( packet) {
                  if( packet.cmd == "CE\r") {
                      if( packet.response[0] == 'E') {
                          var value = packet.response.substr(1);
                          number_calibration  = parseInt(value, 10);
                          $('#cnt_calibration input').val(number_calibration.toString());
                      }
                  }
                  else if( packet.cmd == "GS\r") {
                      var value = packet.response.substr(1);
                      $('#val_GS input').val(parseInt(value, 10).toString());
                  }
                  else if( packet.cmd == "GW\r") {
                       var netto = packet.response.substr(1, 7);
                       var brutto = packet.response.substr(7, 6);
                       var status = packet.response[14];
                       sttus = Number( status);

                       print_log( netto + ":" + brutto + ":" + status);
//parseFloat("554,20".replace(",", "."));
                       $('#Net').val(parseFloat(netto).toString());
                       $('#Brutto').val(parseFloat(brutto).toString());
                       if( !(status & 1)) {
                           $('#isMotion').css("background-color", "#f00");
                       }
                       else { //if( status == 1)
                           $('#isMotion').css("background-color", "#0f0");
                       }
                   }
        }

	function on_calibration_cmd_response( message)
	{
//		print_log( message.payloadString.length);
		var parse_payload = JSON.parse( message.payloadString);
		if( parse_payload) {
              if( currentCmd.mode == 'E') {
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
			else if(	currentCmd.mode == 'M') {
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
            else if( currentCmd.mode == 'C') {
                 if( parse_payload.ret_code == "OK") {
                     parse_packet( parse_payload);


                                currentCmd = {};
                                idxCurrentParameters++;
             	                setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds
                        }
                        }
                        else if( currentCmd.mode == 'A') {
				if( parse_payload.ret_code == "OK") {
                                    parse_packet( parse_payload);
/*
                                    if( parse_payload.cmd == "CE\r") {
                                        var value = parse_payload.response.substr(1);
                                        number_calibration  = parseInt(value, 10);
                                        $('#cnt_calibration input').val(number_calibration.toString());
                                    }
*/
                                    if( parse_payload.cmd == "CM 1\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_CM1 input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "CM 2\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_CM2 input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "CM 3\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_CM3 input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "CI\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_CI input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "MR\r") {
                                        var value = parseInt(parse_payload.response.substr(1));
                                        $('#multi_range').val( value);
                                    }
                                    else if( parse_payload.cmd == "DS\r") {
                                        var value = parseInt(parse_payload.response.substr(1));
                                        $('#display_step').val( value);
                                    }
                                    else if( parse_payload.cmd == "DP\r") {
                                        var value = parseInt(parse_payload.response.substr(1));
                                        $('#decimal_point').val( value);
                                    }
                                    else if( parse_payload.cmd == "CG\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_CG input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "ZT\r") {
                                        var value = parse_payload.response.substr(2);
                                        $('#val_ZT input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "ZR\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_ZR input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "ZI\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_ZI input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "NT\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_NT input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "NR\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#val_NR input').val(parseInt(value, 10).toString());
                                    }
                                    else if( parse_payload.cmd == "FM\r") {
                                        filter_type = parseInt(parse_payload.response.substr(1));

                                        $('#filter_mode').val( filter_type);
                                        setFilterMode( filter_type);
                                    }
                                    else if( parse_payload.cmd == "FL\r") {
                                        var value = parse_payload.response.substr(1);
                                        $('#filter_value').val(parseInt(value, 10));
                                    }

                                }
                                currentCmd = {};
                                idxParameters++;
                                if( idxParameters < commands.length) {
			            setTimeout( getParameters, TIMEOUT_REQUEST); // milliseconds
                                }
                                else {
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

/*
		selectedValue = 0;
		var json = { "cmd": "channel" , "parameter": "" + selectedValue}
		var str_json = JSON.stringify(json);
		client.publish(topic_cmd, str_json, 1, false)
*/		
	}

// Create a client instance
	var client = new Paho.Client(mqtt.hostname, Number(mqtt.port), "calibration-client-id");

// set callback handlers
	client.onConnectionLost = onConnectionLost;
	client.onMessageArrived = onMessageArrived;

// connect the client
	print_log("attempting to connect...")
	client.connect({onSuccess:onConnect, useSSL: false, userName:"matrix", password:"1234"});	
	
	
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

	function getParameters() {
		if( selectedChan == -1)
			return;
                if (Object.keys(currentCmd).length == 0) {
                   if( idxParameters <  commands.length) {
		       var json = { "chan": selectedChan , "cmd": commands[idxParameters] + '\r'}
		       var str_json = JSON.stringify(json);
		       client.publish(topic_cmd, str_json, 1, false);
		       currentCmd = json;
		       currentCmd.mode = 'A';
                   }
                }
                else
      			setTimeout( getParameters, TIMEOUT_REQUEST); // milliseconds

	}

	function getCurrentParameters() {
		if( selectedChan == -1)
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
		       var json = { "chan": selectedChan , "cmd": current_commands[idxCurrentParameters] + '\r'}
		       var str_json = JSON.stringify(json);
		       client.publish(topic_cmd, str_json, 1, false);
		       currentCmd = json;
		       currentCmd.mode = 'C';
                }
                else
                	setTimeout( getCurrentParameters, TIMEOUT_REQUEST); // milliseconds

	}

	$("#device").on('change', function() {
        selectedChan = $(this).val();
		print_log("selectedChan: " + selectedChan);
		if( selectedChan == -1) {
			var json = { "chan": "-1" , "cmd": ""}
			var str_json = JSON.stringify(json);
			client.publish(topic_cmd, str_json, 1, false);
		}
		else {
// Запуск последовательного опроса LDU
			idxParameters = 0;
			setTimeout( getParameters, TIMEOUT_REQUEST); // milliseconds
		}
	});	

        function sendManual() {

		if( selectedChan == -1)
			return;

                if(  Object.keys(currentCmd).length != 0) {
                     setTimeout( sendManual, 10);
                     requestManual = 1;
                     return;
                }

               requestManual = 0;

		var cmd = $("#i_CommandRequest").val();
		print_log(cmd);
		var json = { "chan": selectedChan , "cmd": cmd + '\r'}
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
                          setTimeout( exeCommand, 10);
                          return;
                     }
		     var json = { "chan": selectedChan , "cmd": exec_command[idxExecCommand] + '\r'}
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
          if( selectedChan == -1)
              return;
          exec_command = command_CZ;
          exec_command[0] = exec_command[0] + " " + number_calibration;
          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);
	});

        $("#b_CG").on('click', function() {
          if( selectedChan == -1)
              return;
          exec_command = command_CG;
          exec_command[0] = exec_command[0] + " " + number_calibration;
          exec_command[1] = exec_command[1] + " " + $('#val_CG input').val();
          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);
	});

          $("#b_SaveCalibration").on('click', function() {
          if( selectedChan == -1)
              return;
          exec_command = command_CS;
          exec_command[0] = exec_command[0] + " " + number_calibration;
          disable_work = 1;
          idxExecCommand=0;
          setTimeout( execCommand, TIMEOUT_REQUEST);
	});

});
