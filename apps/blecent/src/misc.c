/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "blecent.h"

/**
 * Utility function to log an array of bytes.
 */
void
print_bytes(const uint8_t *bytes, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        BLECENT_LOG(DEBUG, "%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

char *
addr_str(const void *addr)
{
    static char buf[6 * 2 + 5 + 1];
    const uint8_t *u8p;

    u8p = addr;
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
            u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    return buf;
}

void
print_uuid(const void *uuid128)
{
    uint16_t uuid16;
    const uint8_t *u8p;

    uuid16 = ble_uuid_128_to_16(uuid128);
    if (uuid16 != 0) {
        BLECENT_LOG(DEBUG, "0x%04x", uuid16);
        return;
    }

    u8p = uuid128;

    /* 00001101-0000-1000-8000-00805f9b34fb */
    BLECENT_LOG(DEBUG, "%02x%02x%02x%02x-", u8p[15], u8p[14], u8p[13],
                u8p[12]);
    BLECENT_LOG(DEBUG, "%02x%02x-%02x%02x-", u8p[11], u8p[10], u8p[9], u8p[8]);
    BLECENT_LOG(DEBUG, "%02x%02x%02x%02x%02x%02x%02x%02x",
                   u8p[7], u8p[6], u8p[5], u8p[4],
                   u8p[3], u8p[2], u8p[1], u8p[0]);
}

/**
 * Logs information about a connection to the console.
 */
void
print_conn_desc(const struct ble_gap_conn_desc *desc)
{
    BLECENT_LOG(DEBUG, "handle=%d our_ota_addr_type=%d our_ota_addr=%s ",
                desc->conn_handle, desc->our_ota_addr_type,
                addr_str(desc->our_ota_addr));
    BLECENT_LOG(DEBUG, "our_id_addr_type=%d our_id_addr=%s ",
                desc->our_id_addr_type, addr_str(desc->our_id_addr));
    BLECENT_LOG(DEBUG, "peer_ota_addr_type=%d peer_ota_addr=%s ",
                desc->peer_ota_addr_type, addr_str(desc->peer_ota_addr));
    BLECENT_LOG(DEBUG, "peer_id_addr_type=%d peer_id_addr=%s ",
                desc->peer_id_addr_type, addr_str(desc->peer_id_addr));
    BLECENT_LOG(DEBUG, "conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}


void
print_adv_fields(const struct ble_hs_adv_fields *fields)
{
    char s[BLE_HCI_MAX_ADV_DATA_LEN];
    const uint8_t *u8p;
    int i;

    if (fields->flags_is_present) {
        BLECENT_LOG(DEBUG, "    flags=0x%02x\n", fields->flags);
    }

    if (fields->uuids16 != NULL) {
        BLECENT_LOG(DEBUG, "    uuids16(%scomplete)=",
                    fields->uuids16_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids16; i++) {
            BLECENT_LOG(DEBUG, "0x%04x ", fields->uuids16[i]);
        }
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->uuids32 != NULL) {
        BLECENT_LOG(DEBUG, "    uuids32(%scomplete)=",
                    fields->uuids32_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids32; i++) {
            BLECENT_LOG(DEBUG, "0x%08x ", fields->uuids32[i]);
        }
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->uuids128 != NULL) {
        BLECENT_LOG(DEBUG, "    uuids128(%scomplete)=",
                    fields->uuids128_is_complete ? "" : "in");
        u8p = fields->uuids128;
        for (i = 0; i < fields->num_uuids128; i++) {
            print_uuid(u8p);
            BLECENT_LOG(DEBUG, " ");
            u8p += 16;
        }
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->name != NULL) {
        assert(fields->name_len < sizeof s - 1);
        memcpy(s, fields->name, fields->name_len);
        s[fields->name_len] = '\0';
        BLECENT_LOG(DEBUG, "    name(%scomplete)=%s\n",
                    fields->name_is_complete ? "" : "in", s);
    }

    if (fields->tx_pwr_lvl_is_present) {
        BLECENT_LOG(DEBUG, "    tx_pwr_lvl=%d\n", fields->tx_pwr_lvl);
    }

    if (fields->device_class != NULL) {
        BLECENT_LOG(DEBUG, "    device_class=");
        print_bytes(fields->device_class, BLE_HS_ADV_DEVICE_CLASS_LEN);
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->slave_itvl_range != NULL) {
        BLECENT_LOG(DEBUG, "    slave_itvl_range=");
        print_bytes(fields->slave_itvl_range, BLE_HS_ADV_SLAVE_ITVL_RANGE_LEN);
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->svc_data_uuid16 != NULL) {
        BLECENT_LOG(DEBUG, "    svc_data_uuid16=");
        print_bytes(fields->svc_data_uuid16, fields->svc_data_uuid16_len);
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->public_tgt_addr != NULL) {
        BLECENT_LOG(DEBUG, "    public_tgt_addr=");
        u8p = fields->public_tgt_addr;
        for (i = 0; i < fields->num_public_tgt_addrs; i++) {
            BLECENT_LOG(DEBUG, "public_tgt_addr=%s ", addr_str(u8p));
            u8p += BLE_HS_ADV_PUBLIC_TGT_ADDR_ENTRY_LEN;
        }
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->appearance_is_present) {
        BLECENT_LOG(DEBUG, "    appearance=0x%04x\n", fields->appearance);
    }

    if (fields->adv_itvl_is_present) {
        BLECENT_LOG(DEBUG, "    adv_itvl=0x%04x\n", fields->adv_itvl);
    }

    if (fields->le_addr != NULL) {
        BLECENT_LOG(DEBUG, "    le_addr=%s\n", addr_str(fields->le_addr));
    }

    if (fields->le_role_is_present) {
        BLECENT_LOG(DEBUG, "    le_role=0x%02x\n", fields->le_role);
    }

    if (fields->svc_data_uuid32 != NULL) {
        BLECENT_LOG(DEBUG, "    svc_data_uuid32=");
        print_bytes(fields->svc_data_uuid32, fields->svc_data_uuid32_len);
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->svc_data_uuid128 != NULL) {
        BLECENT_LOG(DEBUG, "    svc_data_uuid128=");
        print_bytes(fields->svc_data_uuid128, fields->svc_data_uuid128_len);
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->uri != NULL) {
        BLECENT_LOG(DEBUG, "    uri=");
        print_bytes(fields->uri, fields->uri_len);
        BLECENT_LOG(DEBUG, "\n");
    }

    if (fields->mfg_data != NULL) {
        BLECENT_LOG(DEBUG, "    mfg_data=");
        print_bytes(fields->mfg_data, fields->mfg_data_len);
        BLECENT_LOG(DEBUG, "\n");
    }
}