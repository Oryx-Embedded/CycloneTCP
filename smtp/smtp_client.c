/**
 * @file smtp_client.c
 * @brief SMTP client (Simple Mail Transfer Protocol)
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @section Description
 *
 * SMTP is designed as a mail transport and delivery protocol. Refer to
 * the following RFCs for complete details:
 * - RFC 5321: Simple Mail Transfer Protocol
 * - RFC 4954: SMTP Service Extension for Authentication
 * - RFC 3207: SMTP Service Extension for Secure SMTP over TLS
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.8.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL SMTP_TRACE_LEVEL

//Dependencies
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "core/net.h"
#include "smtp/smtp_client.h"
#include "core/socket.h"
#include "str.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (SMTP_CLIENT_SUPPORT == ENABLED)


/**
 * @brief Send a mail to the specified recipients
 * @param[in] authInfo Authentication information
 * @param[in] mail Mail contents
 * @return Error code
 **/

error_t smtpSendMail(const SmtpAuthInfo *authInfo, const SmtpMail *mail)
{
   error_t error;
   uint_t i;
   uint_t replyCode;
   IpAddr serverIpAddr;
   SmtpClientContext *context;

   //Check parameters
   if(authInfo == NULL || mail == NULL)
      return ERROR_INVALID_PARAMETER;
   //Make sure the server name is valid
   if(authInfo->serverName == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Sending a mail to %s port %" PRIu16 "...\r\n",
      authInfo->serverName, authInfo->serverPort);

   //The specified SMTP server can be either an IP or a host name
   error = getHostByName(authInfo->interface,
      authInfo->serverName, &serverIpAddr, 0);
   //Unable to resolve server name?
   if(error)
      return ERROR_NAME_RESOLUTION_FAILED;

   //Allocate a memory buffer to hold the SMTP client context
   context = osAllocMem(sizeof(SmtpClientContext));
   //Failed to allocate memory?
   if(context == NULL)
      return ERROR_OUT_OF_MEMORY;

   //Open a TCP socket
   context->socket = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   //Failed to open socket?
   if(!context->socket)
   {
      //Free previously allocated resources
      osFreeMem(context);
      //Report an error
      return ERROR_OPEN_FAILED;
   }

#if (SMTP_CLIENT_TLS_SUPPORT == ENABLED)
   //Do not use SSL/TLS for the moment
   context->tlsContext = NULL;
#endif

   //Start of exception handling block
   do
   {
      //Bind the socket to a particular network interface?
      if(authInfo->interface)
      {
         //Associate the socket with the relevant interface
         error = socketBindToInterface(context->socket, authInfo->interface);
         //Any error to report?
         if(error)
            break;
      }

      //Set timeout for blocking operations
      error = socketSetTimeout(context->socket, SMTP_CLIENT_DEFAULT_TIMEOUT);
      //Any error to report?
      if(error)
         break;

      //Connect to the SMTP server
      error = socketConnect(context->socket, &serverIpAddr, authInfo->serverPort);
      //Connection to server failed?
      if(error)
         break;

#if (SMTP_CLIENT_TLS_SUPPORT == ENABLED)
      //Open a secure SSL/TLS session?
      if(authInfo->useTls)
      {
         //Initialize TLS context
         context->tlsContext = tlsInit();
         //Initialization failed?
         if(context->tlsContext == NULL)
         {
            //Unable to allocate memory
            error = ERROR_OUT_OF_MEMORY;
            //Stop immediately
            break;
         }

         //Bind TLS to the relevant socket
         error = tlsSetSocket(context->tlsContext, context->socket);
         //Any error to report?
         if(error)
            break;

         //Select client operation mode
         error = tlsSetConnectionEnd(context->tlsContext, TLS_CONNECTION_END_CLIENT);
         //Any error to report?
         if(error)
            break;

         //Set the PRNG algorithm to be used
         error = tlsSetPrng(context->tlsContext, authInfo->prngAlgo, authInfo->prngContext);
         //Any error to report?
         if(error)
            break;

         //Perform TLS handshake
         error = tlsConnect(context->tlsContext);
         //Failed to established a TLS session?
         if(error)
            break;
      }
#endif

      //Wait for the connection greeting reply
      error = smtpSendCommand(context, NULL, &replyCode, NULL);
      //Any communication error to report?
      if(error)
         break;

      //Check whether the greeting message was properly received
      if(!SMTP_REPLY_CODE_2YZ(replyCode))
      {
         //An unexpected response was received...
         error = ERROR_UNEXPECTED_RESPONSE;
         //Stop immediately
         break;
      }

      //Clear security features
      context->authLoginSupported = FALSE;
      context->authPlainSupported = FALSE;
      context->authCramMd5Supported = FALSE;
      context->startTlsSupported = FALSE;

      //Send EHLO command and parse server response
      error = smtpSendCommand(context, "EHLO [127.0.0.1]\r\n",
         &replyCode, smtpEhloReplyCallback);
      //Any communication error to report?
      if(error)
         break;

      //Check SMTP response code
      if(!SMTP_REPLY_CODE_2YZ(replyCode))
      {
         //An unexpected response was received...
         error = ERROR_UNEXPECTED_RESPONSE;
         //Stop immediately
         break;
      }

#if (SMTP_CLIENT_TLS_SUPPORT == ENABLED)
      //Check whether the STARTTLS command is supported
      if(context->startTlsSupported && !context->tlsContext)
      {
         //Send STARTTLS command
         error = smtpSendCommand(context, "STARTTLS\r\n", &replyCode, NULL);
         //Any communication error to report?
         if(error)
            break;

         //Check SMTP response code
         if(!SMTP_REPLY_CODE_2YZ(replyCode))
         {
            //An unexpected response was received...
            error = ERROR_UNEXPECTED_RESPONSE;
            //Stop immediately
            break;
         }

         //Initialize TLS context
         context->tlsContext = tlsInit();
         //Initialization failed?
         if(context->tlsContext == NULL)
         {
            //Unable to allocate memory
            error = ERROR_OUT_OF_MEMORY;
            //Stop immediately
            break;
         }

         //Bind TLS to the relevant socket
         error = tlsSetSocket(context->tlsContext, context->socket);
         //Any error to report?
         if(error)
            break;

         //Select client operation mode
         error = tlsSetConnectionEnd(context->tlsContext, TLS_CONNECTION_END_CLIENT);
         //Any error to report?
         if(error)
            break;

         //Set the PRNG algorithm to be used
         error = tlsSetPrng(context->tlsContext, authInfo->prngAlgo, authInfo->prngContext);
         //Any error to report?
         if(error)
            break;

         //Perform TLS handshake
         error = tlsConnect(context->tlsContext);
         //Failed to established a TLS session?
         if(error)
            break;

         //Clear security features
         context->authLoginSupported = FALSE;
         context->authPlainSupported = FALSE;
         context->authCramMd5Supported = FALSE;

         //Send EHLO command and parse server response
         error = smtpSendCommand(context, "EHLO [127.0.0.1]\r\n",
            &replyCode, smtpEhloReplyCallback);
         //Any communication error to report?
         if(error)
            break;

         //Check SMTP response code
         if(!SMTP_REPLY_CODE_2YZ(replyCode))
         {
            //An unexpected response was received...
            error = ERROR_UNEXPECTED_RESPONSE;
            //Stop immediately
            break;
         }
      }
#endif

      //Authentication requires a valid user name and password
      if(authInfo->userName && authInfo->password)
      {
#if (SMTP_CLIENT_LOGIN_AUTH_SUPPORT == ENABLED)
         //LOGIN authentication mechanism supported?
         if(context->authLoginSupported)
         {
            //Perform LOGIN authentication
            error = smtpSendAuthLogin(context, authInfo);
            //Authentication failed?
            if(error)
               break;
         }
         else
#endif
#if (SMTP_CLIENT_PLAIN_AUTH_SUPPORT == ENABLED)
         //PLAIN authentication mechanism supported?
         if(context->authPlainSupported)
         {
            //Perform PLAIN authentication
            error = smtpSendAuthPlain(context, authInfo);
            //Authentication failed?
            if(error)
               break;
         }
         else
#endif
#if (SMTP_CLIENT_CRAM_MD5_AUTH_SUPPORT == ENABLED)
         //CRAM-MD5 authentication mechanism supported?
         if(context->authCramMd5Supported)
         {
            //Perform CRAM-MD5 authentication
            error = smtpSendAuthCramMd5(context, authInfo);
            //Authentication failed?
            if(error)
               break;
         }
         else
#endif
         //No authentication mechanism supported?
         {
            //Skip authentication step
         }
      }

      //Format the MAIL FROM command (a null return path must be accepted)
      if(mail->from.addr)
         sprintf(context->buffer, "MAIL FROM:<%s>\r\n", mail->from.addr);
      else
         strcpy(context->buffer, "MAIL FROM:<>\r\n");

      //Send the command to the server
      error = smtpSendCommand(context, context->buffer, &replyCode, NULL);
      //Any communication error to report?
      if(error)
         break;

      //Check SMTP response code
      if(!SMTP_REPLY_CODE_2YZ(replyCode))
      {
         //An unexpected response was received...
         error = ERROR_UNEXPECTED_RESPONSE;
         //Stop immediately
         break;
      }

      //Format the RCPT TO command
      for(i = 0; i < mail->recipientCount; i++)
      {
         //Skip recipient addresses that are not valid
         if(!mail->recipients[i].addr)
            continue;

         //Format the RCPT TO command
         sprintf(context->buffer, "RCPT TO:<%s>\r\n", mail->recipients[i].addr);
         //Send the command to the server
         error = smtpSendCommand(context, context->buffer, &replyCode, NULL);
         //Any communication error to report?
         if(error)
            break;

         //Check SMTP response code
         if(!SMTP_REPLY_CODE_2YZ(replyCode))
         {
            //An unexpected response was received...
            error = ERROR_UNEXPECTED_RESPONSE;
            //Stop immediately
            break;
         }
      }

      //Propagate exception if necessary
      if(error)
         break;

      //Send message body
      error = smtpSendData(context, mail);
      //Any error to report?
      if(error)
         break;

      //End of exception handling block
   } while(0);

   //Check status code
   if(error == NO_ERROR ||
      error == ERROR_UNEXPECTED_RESPONSE ||
      error == ERROR_AUTHENTICATION_FAILED)
   {
      //Properly disconnect from the SMTP server
      smtpSendCommand(context, "QUIT\r\n", &replyCode, NULL);
   }

#if (SMTP_CLIENT_TLS_SUPPORT == ENABLED)
   if(context->tlsContext != NULL)
   {
      //Gracefully close SSL/TLS session
      tlsShutdown(context->tlsContext);
      //Release SSL/TLS context
      tlsFree(context->tlsContext);
   }
#endif

   //Close socket
   socketClose(context->socket);
   //Clean up previously allocated resources
   osFreeMem(context);

   //Return status code
   return error;
}


/**
 * @brief Callback function to parse EHLO response
 * @param[in] context SMTP client context
 * @param[in] replyLine Response line
 * @param[in] replyCode Response code
 * @return Error code
 **/

error_t smtpEhloReplyCallback(SmtpClientContext *context,
   char_t *replyLine, uint_t replyCode)
{
   char_t *p;
   char_t *token;

   //The line must be at least 4 characters long
   if(strlen(replyLine) < 4)
      return NO_ERROR;

   //Skip the response code and the separator
   replyLine += 4;

   //Get the first keyword
   token = strtok_r(replyLine, " ", &p);
   //Check whether the response line is empty
   if(token == NULL)
      return ERROR_INVALID_SYNTAX;

   //The AUTH keyword contains a space-separated list of
   //names of available authentication mechanisms
   if(!strcasecmp(token, "AUTH"))
   {
      //Process the rest of the line
      while(1)
      {
         //Get the next keyword
         token = strtok_r(NULL, " ", &p);
         //Unable to find the next token?
         if(token == NULL)
            break;

         //LOGIN authentication mechanism supported?
         if(!strcasecmp(token, "LOGIN"))
            context->authLoginSupported = TRUE;
         //PLAIN authentication mechanism supported?
         else if(!strcasecmp(token, "PLAIN"))
            context->authPlainSupported = TRUE;
         //CRAM-MD5 authentication mechanism supported?
         else if(!strcasecmp(token, "CRAM-MD5"))
            context->authCramMd5Supported = TRUE;
      }
   }
   //The STARTTLS keyword is used to tell the SMTP client
   //that the SMTP server allows use of TLS
   else if(!strcasecmp(token, "STARTTLS"))
   {
      //STARTTLS use is allowed
      context->startTlsSupported = TRUE;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Authentication using LOGIN mechanism
 * @param[in] context SMTP client context
 * @param[in] authInfo Authentication information
 * @return Error code
 **/

error_t smtpSendAuthLogin(SmtpClientContext *context, const SmtpAuthInfo *authInfo)
{
#if (SMTP_CLIENT_LOGIN_AUTH_SUPPORT == ENABLED)
   error_t error;
   uint_t replyCode;

   //Send AUTH LOGIN command
   error = smtpSendCommand(context, "AUTH LOGIN\r\n", &replyCode, NULL);

   //Any communication error to report?
   if(error)
      return error;
   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_3YZ(replyCode))
      return ERROR_AUTHENTICATION_FAILED;

   //Encode the user name with Base64 algorithm
   base64Encode(authInfo->userName, strlen(authInfo->userName), context->buffer, NULL);
   //Add a line feed
   strcat(context->buffer, "\r\n");

   //Send the resulting string
   error = smtpSendCommand(context, context->buffer, &replyCode, NULL);
   //Any communication error to report?
   if(error)
      return error;

   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_3YZ(replyCode))
      return ERROR_AUTHENTICATION_FAILED;

   //Encode the password with Base64 algorithm
   base64Encode(authInfo->password, strlen(authInfo->password), context->buffer, NULL);
   //Add a line feed
   strcat(context->buffer, "\r\n");

   //Send the resulting string
   error = smtpSendCommand(context, context->buffer, &replyCode, NULL);
   //Any communication error to report?
   if(error)
      return error;

   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_2YZ(replyCode))
      return ERROR_AUTHENTICATION_FAILED;

   //Successful authentication
   return NO_ERROR;
#else
   //LOGIN authentication is not supported
   return ERROR_AUTHENTICATION_FAILED;
#endif
}


/**
 * @brief Authentication using PLAIN mechanism
 * @param[in] context SMTP client context
 * @param[in] authInfo Authentication information
 * @return Error code
 **/

error_t smtpSendAuthPlain(SmtpClientContext *context, const SmtpAuthInfo *authInfo)
{
#if (SMTP_CLIENT_PLAIN_AUTH_SUPPORT == ENABLED)
   error_t error;
   uint_t n;
   uint_t replyCode;

   //Authorization identity
   strcpy(context->buffer, authInfo->userName);
   n = strlen(authInfo->userName) + 1;
   //Authentication identity
   strcpy(context->buffer + n, authInfo->userName);
   n += strlen(authInfo->userName) + 1;
   //Password
   strcpy(context->buffer + n, authInfo->password);
   n += strlen(authInfo->password);

   //Base64 encoding
   base64Encode(context->buffer, n, context->buffer2, NULL);
   //Format the AUTH PLAIN command
   sprintf(context->buffer, "AUTH PLAIN %s\r\n", context->buffer2);

   //Send the command to the server
   error = smtpSendCommand(context, context->buffer, &replyCode, NULL);
   //Any communication error to report?
   if(error)
      return error;

   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_2YZ(replyCode))
      return ERROR_AUTHENTICATION_FAILED;

   //Successful authentication
   return NO_ERROR;
#else
   //PLAIN authentication is not supported
   return ERROR_AUTHENTICATION_FAILED;
#endif
}


/**
 * @brief Authentication using CRAM-MD5 mechanism
 * @param[in] context SMTP client context
 * @param[in] authInfo Authentication information
 * @return Error code
 **/

error_t smtpSendAuthCramMd5(SmtpClientContext *context, const SmtpAuthInfo *authInfo)
{
#if (SMTP_CLIENT_CRAM_MD5_AUTH_SUPPORT == ENABLED)
   //Hex conversion table
   static const char_t hexDigit[] =
   {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
   };

   //Local variables
   error_t error;
   uint_t n;
   uint_t replyCode;

   //Alias pointers
   uint8_t *challenge = (uint8_t *) context->buffer2;
   uint8_t *digest = (uint8_t *) context->buffer;
   char_t *textDigest = (char_t *) context->buffer2;

   //Send AUTH CRAM-MD5 command
   error = smtpSendCommand(context, "AUTH CRAM-MD5\r\n", &replyCode, NULL);
   //Any communication error to report?
   if(error)
      return error;

   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_3YZ(replyCode))
      return ERROR_AUTHENTICATION_FAILED;

   //Compute the length of the response
   n = strlen(context->buffer);
   //Unexpected response from the SMTP server?
   if(n <= 4)
      return ERROR_INVALID_SYNTAX;

   //Decrypt the Base64 encoded challenge
   error = base64Decode(context->buffer + 4, n - 4, challenge, &n);
   //Decoding failed?
   if(error)
      return error;

   //Compute HMAC using MD5
   error = hmacCompute(MD5_HASH_ALGO, authInfo->password,
      strlen(authInfo->password), challenge, n, digest);
   //HMAC computation failed?
   if(error)
      return error;

   //Convert the digest to text
   for(n = 0; n < MD5_DIGEST_SIZE; n++)
   {
      //Convert upper nibble
      textDigest[n * 2] = hexDigit[(digest[n] >> 4) & 0x0F];
      //Then convert lower nibble
      textDigest[n * 2 + 1] = hexDigit[digest[n] & 0x0F];
   }

   //Properly terminate the string
   textDigest[MD5_DIGEST_SIZE * 2] = '\0';
   //Concatenate the user name and the text representation of the digest
   sprintf(context->buffer, "%s %s", authInfo->userName, textDigest);
   //Encode the resulting string with Base64 algorithm
   base64Encode(context->buffer, strlen(context->buffer), context->buffer2, NULL);
   //Add a line feed
   strcat(context->buffer2, "\r\n");

   //Transmit the Base64 encoded string
   error = smtpSendCommand(context, context->buffer2, &replyCode, NULL);
   //Any communication error to report?
   if(error)
      return error;

   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_2YZ(replyCode))
      return ERROR_AUTHENTICATION_FAILED;

   //Successful authentication
   return NO_ERROR;
#else
   //CRAM-MD5 authentication is not supported
   return ERROR_AUTHENTICATION_FAILED;
#endif
}


/**
 * @brief Send message body
 * @param[in] context SMTP client context
 * @param[in] mail Mail contents
 * @return Error code
 **/

error_t smtpSendData(SmtpClientContext *context, const SmtpMail *mail)
{
   error_t error;
   bool_t first;
   uint_t i;
   uint_t replyCode;
   char_t *p;

   //Send DATA command
   error = smtpSendCommand(context, "DATA\r\n", &replyCode, NULL);
   //Any communication error to report?
   if(error)
      return error;

   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_3YZ(replyCode))
      return ERROR_UNEXPECTED_RESPONSE;

   //Point to the beginning of the buffer
   p = context->buffer;

   //Current date and time
   if(mail->dateTime && mail->dateTime[0] != '\0')
      p += sprintf(p, "Date: %s\r\n", mail->dateTime);

   //Sender address
   if(mail->from.addr)
   {
      //A friendly name may be associated with the sender address
      if(mail->from.name && mail->from.name[0] != '\0')
         p += sprintf(p, "From: \"%s\" <%s>\r\n", mail->from.name, mail->from.addr);
      else
         p += sprintf(p, "From: %s\r\n", mail->from.addr);
   }

   //Recipients
   for(i = 0, first = TRUE; i < mail->recipientCount; i++)
   {
      //Skip recipient addresses that are not valid
      if(!mail->recipients[i].addr)
         continue;

      //Check recipient type
      if(mail->recipients[i].type & SMTP_RCPT_TYPE_TO)
      {
         //The first item of the list requires special processing
         p += sprintf(p, first ? "To: " : ", ");

         //A friendly name may be associated with the address
         if(mail->recipients[i].name && mail->recipients[i].name[0] != '\0')
            p += sprintf(p, "\"%s\" <%s>", mail->recipients[i].name, mail->recipients[i].addr);
         else
            p += sprintf(p, "%s", mail->recipients[i].addr);

         //Prepare to add a new item to the list
         first = FALSE;
      }
   }

   //Properly terminate the line with CRLF
   if(!first)
      p += sprintf(p, "\r\n");

   //Carbon copy
   for(i = 0, first = TRUE; i < mail->recipientCount; i++)
   {
      //Skip recipient addresses that are not valid
      if(!mail->recipients[i].addr)
         continue;

      //Check recipient type
      if(mail->recipients[i].type & SMTP_RCPT_TYPE_CC)
      {
         //The first item of the list requires special processing
         p += sprintf(p, first ? "Cc: " : ", ");

         //A friendly name may be associated with the address
         if(mail->recipients[i].name && mail->recipients[i].name[0] != '\0')
            p += sprintf(p, "\"%s\" <%s>", mail->recipients[i].name, mail->recipients[i].addr);
         else
            p += sprintf(p, "%s", mail->recipients[i].addr);

         //Prepare to add a new item to the list
         first = FALSE;
      }
   }

   //Properly terminate the line with CRLF
   if(!first)
      p += sprintf(p, "\r\n");

   //Subject
   if(mail->subject)
      p += sprintf(p, "Subject: %s\r\n", mail->subject);

   //The header and the body are separated by an empty line
   sprintf(p, "\r\n");

   //Debug message
   TRACE_DEBUG(context->buffer);
   TRACE_DEBUG(mail->body);
   TRACE_DEBUG("\r\n.\r\n");

   //Send message header
   error = smtpWrite(context, context->buffer, strlen(context->buffer), 0);
   //Any communication error to report?
   if(error)
      return error;

   //Send message body
   error = smtpWrite(context, mail->body, strlen(mail->body), 0);
   //Any communication error to report?
   if(error)
      return error;

   //Indicate the end of the mail data by sending a line containing only a "."
   error = smtpSendCommand(context, "\r\n.\r\n", &replyCode, NULL);
   //Any communication error to report?
   if(error)
      return error;

   //Check SMTP reply code
   if(!SMTP_REPLY_CODE_2YZ(replyCode))
      return ERROR_UNEXPECTED_RESPONSE;

   //Successful operation
   return NO_ERROR;
}


/**
 * @brief Send SMTP command and wait for a reply
 * @param[in] context SMTP client context
 * @param[in] command Command line
 * @param[out] replyCode SMTP server reply code
 * @param[in] callback Optional callback to parse each line of the reply
 * @return Error code
 **/

error_t smtpSendCommand(SmtpClientContext *context, const char_t *command,
   uint_t *replyCode, SmtpReplyCallback callback)
{
   error_t error;
   size_t length;
   char_t *line;

   //Any command line to send?
   if(command)
   {
      //Debug message
      TRACE_DEBUG("SMTP client: %s", command);

      //Send the command to the SMTP server
      error = smtpWrite(context, command, strlen(command), SOCKET_FLAG_WAIT_ACK);
      //Failed to send command?
      if(error)
         return error;
   }

   //Multiline replies are allowed for any command
   while(1)
   {
      //Wait for a response from the server
      error = smtpRead(context, context->buffer,
         SMTP_CLIENT_MAX_LINE_LENGTH - 1, &length, SOCKET_FLAG_BREAK_CRLF);

      //The remote server did not respond as expected?
      if(error)
         return error;

      //Properly terminate the string with a NULL character
      context->buffer[length] = '\0';
      //Remove all leading and trailing whitespace from the response
      line = strTrimWhitespace(context->buffer);

      //Debug message
      TRACE_DEBUG("SMTP server: %s\r\n", line);

      //Check the length of the response
      if(strlen(line) < 3)
         return ERROR_INVALID_SYNTAX;
      //All replies begin with a three digit numeric code
      if(!isdigit((uint8_t) line[0]) || !isdigit((uint8_t) line[1]) || !isdigit((uint8_t) line[2]))
         return ERROR_INVALID_SYNTAX;
      //A hyphen or a space character must follow the response code
      if(line[3] != '-' && line[3] != ' ' && line[3] != '\0')
         return ERROR_INVALID_SYNTAX;

      //Get the server response code
      *replyCode = strtoul(line, NULL, 10);

      //Any callback function to call?
      if(callback)
      {
         //Invoke callback function to parse the response line
         error = callback(context, line, *replyCode);
         //Check status code
         if(error)
            return error;
      }

      //A hyphen follows the response code for all but the last line
      if(line[3] != '-')
         break;
   }

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Send data to the SMTP server
 * @param[in] context SMTP client context
 * @param[in] data Pointer to a buffer containing the data to be transmitted
 * @param[in] length Number of bytes to be transmitted
 * @param[in] flags Set of flags that influences the behavior of this function
 **/

error_t smtpWrite(SmtpClientContext *context, const void *data, size_t length, uint_t flags)
{
#if (SMTP_CLIENT_TLS_SUPPORT == ENABLED)
   //Check whether a secure connection is being used
   if(context->tlsContext != NULL)
   {
      //Use SSL/TLS to transmit data to the SMTP server
      return tlsWrite(context->tlsContext, data, length, NULL, flags);
   }
   else
#endif
   {
      //Transmit data to the SMTP server
      return socketSend(context->socket, data, length, NULL, flags);
   }
}


/**
 * @brief Receive data from the SMTP server
 * @param[in] context SMTP client context
 * @param[out] data Buffer into which received data will be placed
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] received Actual number of bytes that have been received
 * @param[in] flags Set of flags that influences the behavior of this function
 * @return Error code
 **/

error_t smtpRead(SmtpClientContext *context, void *data, size_t size, size_t *received, uint_t flags)
{
#if (SMTP_CLIENT_TLS_SUPPORT == ENABLED)
   //Check whether a secure connection is being used
   if(context->tlsContext != NULL)
   {
      //Use SSL/TLS to receive data from the SMTP server
      return tlsRead(context->tlsContext, data, size, received, flags);
   }
   else
#endif
   {
      //Receive data from the SMTP server
      return socketReceive(context->socket, data, size, received, flags);
   }
}

#endif
