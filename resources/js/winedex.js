/*
	Thomas Lextrait
	tlextrai@brandeis.edu
	CS132 - Information Retrieval
	Project Wine
*/

var Wine = {

	// This is the server endpoint
	server_endpoint: "/",
	api_search: "search",
	api_find_similar: "similar",
	no_img: "images/empty_glass.jpg",

	// Make a search query
	search: function(){

		Wine.showLoading();

		$.get(Wine.server_endpoint + Wine.api_search, {
			"q": $("#query").val()
		})
		.fail(function(){
			Wine.showError();
		})
		.done(function(data){

			// Clear results
			Wine.clearResults();

			var json = $.parseJSON(data);

			if(json.results){

				// Print stats
				Wine.printInfoStats(json);

				if(json.results.length > 0){	
					Wine.printResults(json);
				}else{
					Wine.showNoResults(json);
				}
			}else{
				Wine.showError();
			}
			
		});

	},

	/**
	* Makes a search programmatically
	* @param {string} query string
	*/
	q: function(query){
		$("#query").val(decodeURIComponent(query));
		Wine.search();
	},

	/**
	* Makes a query for finding similar wine bottles
	* @param {string} wine code
	*/
	findSimilar: function(wine_code){

		Wine.showLoading();

		$.get(Wine.server_endpoint + Wine.api_find_similar, {
			"wine":wine_code
		})
		.fail(function(){
			Wine.showError();
		})
		.done(function(data){

			// Clear results
			Wine.clearResults();

			var json = $.parseJSON(data);

			if(json.results){

				// Print stats
				Wine.printInfoStats(json);

				if(json.results.length > 0){	
					Wine.printResults(json);
				}else{
					Wine.showNoResults(json);
				}
			}else{
				Wine.showError();
			}

		});
	},

	/**
	* Print search result stats
	*/
	printInfoStats: function(json){

		var info = $("<div id='stats'></div>");

		// Total results
		if(json.total_results){
			info.append("<span>"+json.total_results+" results</span>");
		}

		// Print time
		if(json.total_time){
			info.append("<span>completed in "+json.total_time+"ms</span>");
		}

		// Print suggestions
		if(json.query_suggestions && json.query_suggestions.length > 0){
			var sugg = "";
			var first = true;
			$.each(json.query_suggestions, function(key, item){
				if(!first){sugg += ", ";}else{first = false;}
				sugg += "<a href='javascript:Wine.q(\""+encodeURIComponent(item)+"\")'>"+item+"</a>";
			});
			info.append("<span class='suggestions'>Did you mean: "+sugg+"</span>");
		}

		var center = $("<center></center>");
		center.append(info);

		$("#results").append(center);
	},

	/**
	* Print the search results
	*/
	printResults: function(json){
		// Print results
		$.each(json.results, function(key, item){

			// Image
			var image = item.image && item.image!="" ? item.image : Wine.no_img;

			// Region
			var region = item.region && item.region!="" ? ", "+item.region : "";

			// Winery
			var winery = item.winery && item.winery!="" ? "<span class='winery'>"+item.winery+region+"</span>" : "";

			// Vintage
			var vintage = item.vintage && item.vintage!="" ? "<span class='label vintage'>"+item.vintage+"</span>" : "";

			// Type
			var type = item.type && item.type!="" ? "<span class='label vintage'>"+item.type+"</span>" : "";

			// Snooth rank
			var snooth = item.snoothrank && item.snoothrank!="" ? "<span class='label critics'>Snooth "+item.snoothrank+"/5</span>" : "";

			// Critic scores
			var critics = "";
			if(item.critic_scores && item.critic_scores!=""){
				$.each(item.critic_scores, function(key, value){
					critics += "<span class='label critics'>"+value.name+" "+value.raw_score+"</span>";
				});
			}

			// Price
			var price = item.price && item.price!="" && item.price!="0" ? "<span class='label price'>$"+item.price+"</span>" : "";

			// Find similar
			var find_similar = "";
			if(
				item._index_taste_profile.positive == "True" &&
				item._index_taste_similar_count > 1
			){
				find_similar = 
					"<a class='button side' href='javascript:Wine.findSimilar(\""+item.code+"\")'>"+
						"Find similar ("+item._index_taste_similar_count+")"+
					"</a>";
			}

			// Taste button
			var taste_button = "";
			if(item._index_taste_profile.positive == "True"){
				var taste = "";
				$.each(item._index_taste_profile, function(key, item){
					taste += key+":"+item+", ";
				});

				taste_button = "<a class='button side' href='javascript:alert(\""+taste+"\")'>"+
									"Review taste"+
								"</a> ";
			}
			 
		    $("#results").append(
		    	"<div class='wine'>"+
		    		"<table cellpadding='0' cellspacing='0'>"+
		    		"<tr>"+
		    			"<td class='img'><img src='"+image+"'/></td>"+
		    			"<td>"+
		    				"<span class='name'><a href='"+item.link+"' target='_blank'>"+item.name+"</a></span>"+
		    				winery+
		    				"<div class='label_block'>"+vintage+type+snooth+critics+"</div>"+
		    			"</td>"+
		    			"<td style='text-align:right'>"+
		    				price+"<br/>"+taste_button+find_similar+
		    			"</td>"+
		    		"</tr>"+
		    		"</table>"+
		    	"</div>"
		    );
		});
	},

	/**
	* Clears results
	*/
	clearResults: function(){
		$("#results").html("");
	},

	/* ==================================================== */
	/* Display messages										*/
	/* ==================================================== */

	showNoResults: function(json){
		var msg = "<center>No results found"
		if(json.query_suggestions && json.query_suggestions.length > 0){
			msg += ", try:<br/><span class='try_suggestions'>";
			var first = true;
			$.each(json.query_suggestions, function(key, item){
				if(!first){msg += ", ";}else{first = false;}
				msg += "<a href='javascript:Wine.q(\""+encodeURIComponent(item)+"\")'>"+item+"</a>";
			});
			msg += "</span>";
		}else{
			msg += ".";
		}
		msg += "</center>";

		$("#results").html(msg);
	},

	showError: function(){
		$("#results").html("<center>An error occured, server seem unavailable</center>");
	},

	// Show the loading sign
	showLoading: function(){
		$("#results").html("<center>loading...</center>");
	},

};
