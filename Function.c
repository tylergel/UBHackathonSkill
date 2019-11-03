using System;
using System.Collections.Generic;
using Amazon.Lambda.Core;
using Newtonsoft.Json;
using AlexaAPI;
using AlexaAPI.Request;
using AlexaAPI.Response;
using System.IO;
using System.Text.RegularExpressions;
using System.Net;
using System.Text;
using System.Collections.Specialized;

[assembly: LambdaSerializerAttribute(typeof(Amazon.Lambda.Serialization.Json.JsonSerializer))]

namespace sampleFactCsharp
{
    public class Function
    {


        public class WatchlistObject
        {
            public string stock { get; set; }
            public string price { get; set; }
            
        }


        
        private SkillResponse response = null;
        private ILambdaContext context = null;
        const string LOCALENAME = "locale";
        const string USA_Locale = "en-US";

        static Random rand = new Random();

        /// <summary>
        /// Application entry point
        /// </summary>
        /// <param name="input"></param>
        /// <param name="ctx"></param>
        /// <returns></returns>
        public SkillResponse FunctionHandler(SkillRequest input, ILambdaContext ctx)
        {
            context = ctx;
            try
            {
                response = new SkillResponse();
                response.Response = new ResponseBody();
                response.Response.ShouldEndSession = false;
                response.Version = AlexaConstants.AlexaVersion;

                if (input.Request.Type.Equals(AlexaConstants.LaunchRequest))
                {
                    string locale = input.Request.Locale;
                    if (string.IsNullOrEmpty(locale))
                    {
                        locale = USA_Locale;
                    }

                    ProcessLaunchRequest(response.Response);
                    response.SessionAttributes = new Dictionary<string, object>() {{LOCALENAME, locale}};
                }
                else
                {
                    if (input.Request.Type.Equals(AlexaConstants.IntentRequest))
                    {
                       string locale = string.Empty;
                       Dictionary <string, object> dictionary = input.Session.Attributes;
                       if (dictionary != null)
                       {
                           if (dictionary.ContainsKey(LOCALENAME))
                           {
                               locale = (string) dictionary[LOCALENAME];
                           }
                       }
               
                       if (string.IsNullOrEmpty(locale))
                       {
                            locale = input.Request.Locale;
                       }

                       if (string.IsNullOrEmpty(locale))
                       {
                            locale = USA_Locale; 
                       }

                       response.SessionAttributes = new Dictionary<string, object>() {{LOCALENAME, locale}};

                       if (IsDialogIntentRequest(input))
                       {
                            if (!IsDialogSequenceComplete(input))
                            { // delegate to Alexa until dialog is complete
                                CreateDelegateResponse();
                                return response;
                            }
                       }

                       if (!ProcessDialogRequest(input, response))
                       {
                           response.Response.OutputSpeech = ProcessIntentRequest(input);
                       }
                    }
                }
                return response;
            }
            catch (Exception ex)
            {
            }
            return null; 
        }

       
        private void ProcessLaunchRequest(ResponseBody response)
        {
                IOutputSpeech innerResponse = new SsmlOutputSpeech();
                (innerResponse as SsmlOutputSpeech).Ssml = SsmlDecorate("UBHackathon stock watchlist has been launched");
                response.OutputSpeech = innerResponse;
                IOutputSpeech prompt = new PlainTextOutputSpeech();
                (prompt as PlainTextOutputSpeech).Text = "UBHackathon stock watchlist has been launched";
                response.Reprompt = new Reprompt()
                {
                    OutputSpeech = prompt
                };
        }
        private bool IsDialogIntentRequest(SkillRequest input)
        {
            if (string.IsNullOrEmpty(input.Request.DialogState))
                return false;
            return true;
        }

        private bool IsDialogSequenceComplete(SkillRequest input)
        {
            if (input.Request.DialogState.Equals(AlexaConstants.DialogStarted)
               || input.Request.DialogState.Equals(AlexaConstants.DialogInProgress))
            { 
                return false ;
            }
            else
            {
                if (input.Request.DialogState.Equals(AlexaConstants.DialogCompleted))
                {
                    return true;
                }
            }
            return false;
        }
        public WatchlistObject GetWatchlist(string customer)
        {
			return null;
        }

        private bool ProcessDialogRequest(SkillRequest input, SkillResponse response)
        {
            var intentRequest = input.Request;
            string speech_message = string.Empty;
            bool processed = false;

            switch (intentRequest.Intent.Name)
            {

                case "GetWatchlist":


					WebRequest req = HttpWebRequest.Create("https://webshockinnovations.com/ubhackathonapi/api.php");
						WebResponse res = req.GetResponse();
					StreamReader reader = new StreamReader(res.GetResponseStream());

					string json = reader.ReadToEnd();

					List<WatchlistObject> watchlist = Newtonsoft.Json.JsonConvert.DeserializeObject<List<WatchlistObject>>(json);
					string stocks = "";
					foreach(WatchlistObject watch  in watchlist)
					{
						stocks += watch.stock + ", ";
					}
					speech_message = "Your watchlist contains " + watchlist.Count.ToString() + " stocks.  They are: " + stocks;
                    if (!string.IsNullOrEmpty(speech_message))
                    {
                        response.Response.OutputSpeech = new SsmlOutputSpeech();
                        (response.Response.OutputSpeech as SsmlOutputSpeech).Ssml = SsmlDecorate(speech_message);
                    }
                    processed = true;
                    break;



            }


            return processed;
        }
        private string SsmlDecorate(string speech)
        {
            return "<speak>" + speech + "</speak>";
        }

        private IOutputSpeech ProcessIntentRequest(SkillRequest input)
        {
            var intentRequest = input.Request;
            IOutputSpeech innerResponse = new PlainTextOutputSpeech();
            
            switch (intentRequest.Intent.Name)
            {
               
                case AlexaConstants.CancelIntent:
                    (innerResponse as PlainTextOutputSpeech).Text = "";
                    response.Response.ShouldEndSession = true;
                    break;

                case AlexaConstants.StopIntent:
                    (innerResponse as PlainTextOutputSpeech).Text = "";
                    response.Response.ShouldEndSession = true;                    
                    break;

                case AlexaConstants.HelpIntent:
                    (innerResponse as PlainTextOutputSpeech).Text = "";
                    break;



                default:
                    (innerResponse as PlainTextOutputSpeech).Text = ""; 
                    break;
            }
            if (innerResponse.Type == AlexaConstants.SSMLSpeech)
            {
               
                (innerResponse as SsmlOutputSpeech).Ssml = "";
            }  
            return innerResponse;
        }




        private void CreateDelegateResponse()
        {
            DialogDirective dld = new DialogDirective()
            {
                Type = AlexaConstants.DialogDelegate
            };
            response.Response.Directives.Add(dld);
        }

    }
}
