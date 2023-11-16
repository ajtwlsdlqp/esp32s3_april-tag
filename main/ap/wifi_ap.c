/*
 * wifi_ap.c
 *
 *  Created on: 2021. 1. 27.
 *      Author: HanCheol Cho
 */


#include "hw.h"
#include "wifi_ap.h"
#include "ap.h"

#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "uart.h"
#include "qbuffer.h"

static const char* TAG = "camera";
#define CAM_USE_WIFI

#define ESP_WIFI_SSID "cam"
#define ESP_WIFI_PASS ""

#define MAX_STA_CONN  5

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

#define PORT                        3333
#define KEEPALIVE_IDLE              5
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             3

typedef struct
{
    size_t size;  //number of values used for filtering
    size_t index; //current value index
    size_t count; //value count
    int sum;
    int *values; //array to be filled with values
} ra_filter_t;

static ra_filter_t ra_filter;
extern airb_info_t info;

void wifi_init_softap();
esp_err_t http_server_init(httpd_handle_t *p_server, httpd_config_t *p_config);
static int ra_filter_run(ra_filter_t *filter, int value);
static esp_err_t jpg_stream_httpd_handler(httpd_req_t *req);
static esp_err_t jpg_socket_httpd_handler(httpd_req_t *req);

static int sock = 0;
uint32_t last_socket_time;

void wifiApInit(void)
{
  esp_err_t err = nvs_flash_init();

  if (err != ESP_OK)
  {
    ESP_ERROR_CHECK( nvs_flash_erase() );
    ESP_ERROR_CHECK( nvs_flash_init() );
  }


  info.fb = NULL;
  info.is_valid = false;

  wifi_init_softap();
  delay(100);

  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  http_server_init(&server, &config);
}

int ra_filter_run(ra_filter_t *filter, int value)
{
  if (!filter->values)
  {
      return value;
  }
  filter->sum -= filter->values[filter->index];
  filter->values[filter->index] = value;
  filter->sum += filter->values[filter->index];
  filter->index++;
  filter->index = filter->index % filter->size;
  if (filter->count < filter->size)
  {
      filter->count++;
  }
  return filter->sum / filter->count;
}

esp_err_t jpg_httpd_handler(httpd_req_t *req)
{
  esp_err_t res = ESP_OK;
  size_t fb_len = 0;
  int64_t fr_start = esp_timer_get_time();

  camera_fb_t *frame = NULL;
  uint8_t *cam_buf = NULL;
  uint32_t length = 0;

  info.webpage_stream = true;
  if (xQueueReceive(info.xQueueFrameO, &frame, portMAX_DELAY))
  {
    if(frame->format != PIXFORMAT_JPEG)
    {
      frame2jpg(frame, 20, &cam_buf, &length);
    }
    else
    {
      cam_buf = frame->buf;
      length = frame->len;
    }


    res = httpd_resp_set_type(req, "image/jpeg");

    if(res == ESP_OK)
    {
      res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    if(res == ESP_OK)
    {
      fb_len = length;
      res = httpd_resp_send(req, (const char *)cam_buf, length);
    }

    esp_camera_fb_return(frame);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    cliPrintf("JPG: %uKB %ums \n", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
  }
  else
  {
    cliPrintf("xQueueReceive Fail\n!");
  }

  info.webpage_stream = false;

  return res;
}

esp_err_t jpg_stream_httpd_handler(httpd_req_t *req)
{
//  struct timeval _timestamp;
  esp_err_t res = ESP_OK;

  char *part_buf[128];

  camera_fb_t *frame = NULL;
  uint8_t *cam_buf = NULL;
  uint32_t length = 0;

  static int64_t last_frame = 0;
  if (!last_frame)
  {
      last_frame = esp_timer_get_time();
  }

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
  {
      return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "25");

  info.webpage_stream = true;
  while (true)
  {
    if (xQueueReceive(info.xQueueFrameO, &frame, portMAX_DELAY))
    {
//      _timestamp.tv_sec = frame->timestamp.tv_sec;
//      _timestamp.tv_usec = frame->timestamp.tv_usec;
      res = ESP_OK;
    }
    else
    {
      res = ESP_FAIL;
    }

    if (res == ESP_OK)
    {
      if(frame->format != PIXFORMAT_JPEG)
      {
        res = frame2jpg(frame, 50, &cam_buf, &length);  // return bool type
        delay(1);
        if(!res)
        {
          cliPrintf(TAG, "JPEG compression failed");
          esp_camera_fb_return(frame);
          res = ESP_FAIL;
        }
        else res = ESP_OK;
      }
      else
      {
        cam_buf = frame->buf;
        length = frame->len;
      }
    }

    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }

    if (res == ESP_OK)
    {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, length);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }

    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, (const char *)cam_buf, length);
    }

    if(frame->format != PIXFORMAT_JPEG && res == ESP_OK)
    {
      free(cam_buf);
    }

    esp_camera_fb_return(frame);

    if (res != ESP_OK)
    {
      break;
    }

    /*
    int64_t fr_end = esp_timer_get_time();
    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);

    cliPrintf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps)\n"
             ,
             (uint32_t)(length),
             (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
             avg_frame_time, 1000.0 / avg_frame_time
    );
    */
    delay(1);
  }

  delay(10);

  info.webpage_stream = false;

  last_frame = 0;
  return res;
}

static void doTxRxSocket(const int sock)
{
  bool socket_destroy = false;
  int len_rx, len_tx;
  char rx_buffer[128];
  uint8_t jpg_header[10]= {0,0,0,0,'S','T','R','E','A','M'}; // (uint16_t)SizeData

  last_socket_time = millis();  // clear
  do
  {
    len_rx = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT);  // Non blocking

    if (len_rx == 0)
    {
      last_socket_time = millis();
      socket_destroy = true;
//      ESP_LOGI("SOCKET", "Connection closed");
    }
    else if( len_rx > 0)
    {
      last_socket_time = millis();
      rx_buffer[len_rx] = 0; // Null-terminate whatever is received and treat it like a string
//      ESP_LOGI("SOCKET", "Received %d bytes: %s", len_rx, rx_buffer);
//      for( int debug=0; debug<len_rx; debug++)
//      {
//        cliPrintf("%02X ", rx_buffer[debug]);
//      }


      if( rx_buffer[0] == 0xFF && rx_buffer[1] == 0xFF && rx_buffer[2] == 0xFD)
      {
        // send to DXL-Packet Decoder
        memcpy(uartGetWrBuffer(), rx_buffer, len_rx); // Uart wr buffer update..(1)
        setSocketUpateLen(len_rx);
      }

      memset(rx_buffer, 0x00, len_rx);
      // send() can return less bytes than supplied length.
      // Walk-around for robust implementation.
    }
    else
    {
      camera_fb_t *frame = NULL;
      len_tx = getSocketUpdateLen();
      if (len_tx > 0)
      {
        int to_write = len_tx;
//        ESP_LOGI("SOCKET", "Transmit %d bytes: %s", len_tx, uartGetRxBuffer() );
//        delay(100);
        while (to_write > 0)
        {
          int written = send(sock, uartGetRxBuffer() + (len_tx - to_write), to_write, 0);
          if (written < 0)
          {
            ESP_LOGE("SOCKET", "Error occurred during sending: errno");
            // Failed to retransmit, giving up
            break;
          }
          to_write -= written;
        }
//        delay(100);
        confirmSocketUpdatedLen();
      }
      else if( info.socket_stream == true)
      {
        data_t jpeg_len;
        int to_send;
        last_socket_time = millis();

        if (xQueueReceive(info.xQueueFrameO, &frame, portMAX_DELAY))
        {
          int sent;
          to_send = 10;
          jpeg_len.u32D = frame->len+6;
          jpg_header[0] = jpeg_len.u8Data[0];
          jpg_header[1] = jpeg_len.u8Data[1];
          jpg_header[2] = jpeg_len.u8Data[2];
          jpg_header[3] = jpeg_len.u8Data[3];

          while( to_send > 0)
          {
            sent = send(sock, jpg_header, to_send, 0);
            if (sent < 0 || info.socket_stream == false)
            {
              ESP_LOGE("SOCKET", "Error occurred during sending: errno");
              info.socket_stream = false;
              // Failed to retransmit, giving up
              esp_camera_fb_return(frame);
              break;
            }
            to_send -= sent;
          }
          if (sent < 0 || info.socket_stream == false) return;

          to_send = frame->len;
//          ESP_LOGI("SOCKET", "JPGE %d bytes", to_send);
          while( to_send > 0)
          {
            sent = send(sock, frame->buf + (frame->len - to_send), to_send, 0);
            if (sent < 0 || info.socket_stream == false)
            {
              ESP_LOGE("SOCKET", "Error occurred during sending: errno");
              info.socket_stream = false;
              // Failed to retransmit, giving up
              esp_camera_fb_return(frame);
              break;
            }
            to_send -= sent;
          }
          if (sent < 0 || info.socket_stream == false) return;
        }
        esp_camera_fb_return(frame);

        if( info.step_shutdown == true)
        {
          info.step_shutdown = false;
          info.socket_stream = false;
        }
      }

      if( millis() - last_socket_time > 5000) // over 5sec...
      {
        ESP_LOGE("SOCKET", "last_socket_time Over...!");
        socket_destroy = true;
      }

    }
  }
  while (socket_destroy == false);
  info.socket_stream = false;
}

static void tcp_server_task(void *pvParameters)
{
  char addr_str[128];
  int addr_family = (int)pvParameters;
  int ip_protocol = 0;
  int keepAlive = 1;
  int keepIdle = KEEPALIVE_IDLE;
  int keepInterval = KEEPALIVE_INTERVAL;
  int keepCount = KEEPALIVE_COUNT;
  struct sockaddr_storage dest_addr;

  if (addr_family == AF_INET)
  {
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);
    ip_protocol = IPPROTO_IP;
  }

  int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
  if (listen_sock < 0)
  {
    ESP_LOGE("SOCKET", "Unable to create socket: errno %d", errno);
    vTaskDelete(NULL);
    return;
  }
  else
  {
    ESP_LOGI("SOCKET", "Aable to create socket");
  }

  int opt = 1;
  setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  ESP_LOGI("SOCKET", "Socket created");

  int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
  if (err != 0)
  {
    ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
    goto CLEAN_UP;
  }
  ESP_LOGI("SOCKET", "Socket bound, port %d", PORT);

  err = listen(listen_sock, 1);
  if (err != 0)
  {
    ESP_LOGE("SOCKET", "Error occurred during listen: errno %d", errno);
    goto CLEAN_UP;
  }

  while (1)
  {
    ESP_LOGI("SOCKET", "Socket listening");

    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t addr_len = sizeof(source_addr);
    sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
    if (sock < 0)
    {
      ESP_LOGE("SOCKET", "Unable to accept connection: errno %d", errno);
      break;
    }

    // Set tcp keepalive option
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
    // Convert ip address to string

    if (source_addr.ss_family == PF_INET)
    {
      inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
    }

    ESP_LOGI("SOCKET", "Socket accepted ip address: %s", addr_str);
    uartOpen(_DEF_UART3, 1000000);

    doTxRxSocket(sock);

    /*
     * 더 이상 보낼 데이터가 없다. FIN 전송
     * 셧다운 된 소켓은 재활용이 불가하며, 다시 생성해야 한다.
     * 어플리케이션의 Write() /  Read() 함수가 동작하지 못하도록 하는 Code
     * #define SHUT_RD   0 / SHUT_WR   1 / SHUT_RDWR 2
     */
    shutdown(sock, SHUT_RD);
    close(sock);        // 서버의 소켓을 닫아버림, FIN에 대한 ACK는 무시
  }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

int getWebSocket(void)
{
  return sock;
}

void setCloseSocket(void)
{
  shutdown(sock, SHUT_WR);
  close(sock);        // 서버의 소켓을 닫아버림, FIN에 대한 ACK는 무시

  //closesocket(sock);
}

esp_err_t http_server_init(httpd_handle_t *p_server, httpd_config_t *p_config)
{
  httpd_uri_t jpeg_uri =
  {
    .uri = "/jpg",
    .method = HTTP_GET,
    .handler = jpg_httpd_handler,
    .user_ctx = NULL
  };

  httpd_uri_t jpeg_stream_uri =
  {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = jpg_stream_httpd_handler,
    .user_ctx = NULL
  };

  ESP_ERROR_CHECK(httpd_start(p_server, p_config));
  ESP_ERROR_CHECK(httpd_register_uri_handler(*p_server, &jpeg_uri));
  ESP_ERROR_CHECK(httpd_register_uri_handler(*p_server, &jpeg_stream_uri));

  xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);

  return ESP_OK;
}


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
  if (event_id == WIFI_EVENT_AP_STACONNECTED)
  {
    wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    logPrintf("station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
  }
  else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
  {
    wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
    logPrintf(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
  }
}

void wifi_init_softap(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      NULL));

  wifi_config_t wifi_config = {
      .ap = {
          .ssid = ESP_WIFI_SSID,
          .ssid_len = strlen(ESP_WIFI_SSID),
          .channel = 2,
          .password = ESP_WIFI_PASS,
          .max_connection = MAX_STA_CONN,
          .authmode = WIFI_AUTH_WPA_WPA2_PSK
      },
  };
  if (strlen(ESP_WIFI_PASS) == 0)
  {
      wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  cliPrintf("wifi_init_softap finished. SSID:%s password:%s channel:%d\n",
           ESP_WIFI_SSID, ESP_WIFI_PASS, 2);
}
