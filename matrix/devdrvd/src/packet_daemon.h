#ifndef PACKET_DAEMON_H
#define PACKET_DAEMON_H

/**
 * @brief запускает поток для раздачи данных клиентам
 *
 * @param fd файловый дескриптор
 */
void* runpacket(void*);

#endif // PACKET_DAEMON_H
