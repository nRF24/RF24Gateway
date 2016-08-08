
-- RF24Network LUA dissector for Wireshark
-- Usage: 1. Start wireshark on the host machine running RF24Gateway
--        2. Use the command `wireshark -X lua_script:RF24Network.lua`
--        3. In filters, add '!icmp' and apply
--        4. RF24Network user payloads show as Protocol: RF24Net & header information is displayed

rf24network_proto = Proto("rf24network","RF24Network Protocol")

rf24network_proto.fields.nodeID = ProtoField.uint8("rf24network.nodeID", "Mesh Node", base.DEC)
rf24network_proto.fields.from = ProtoField.uint16("rf24network.from", "Network from Node", base.OCT)
rf24network_proto.fields.to = ProtoField.uint16("rf24network.to", "Network to Node", base.OCT)
rf24network_proto.fields.hID = ProtoField.uint16("rf24network.hID", "Header ID", base.DEC)
rf24network_proto.fields.type = ProtoField.uint8("rf24network.type", "Header Type", base.DEC)
rf24network_proto.fields.reserved = ProtoField.uint8("rf24network.reserved", "Header Reserved", base.DEC)
rf24network_proto.fields.pSize = ProtoField.uint16("rf24network.payloadSize", "Payload Size", base.DEC)


rf24network_proto.fields.uint16_t = ProtoField.uint32("rf24network.uint16", "16-bit unsigned:", base.DEC)
rf24network_proto.fields.int16_t = ProtoField.int32("rf24network.int16", "16-bit signed", base.DEC)
rf24network_proto.fields.uint32_t = ProtoField.uint32("rf24network.uint32", "32-bit unsigned", base.DEC)
rf24network_proto.fields.int32_t = ProtoField.uint32("rf24network.int32", "32-bit signed", base.DEC)


function rf24network_proto.dissector(buffer,pinfo,tree)
    pinfo.cols.protocol = "RF24Net"
    local subtree = tree:add_le(rf24network_proto,buffer(),"RF24Network Header")
    if(buffer:len() >= 11) then
        subtree:add(rf24network_proto.fields.nodeID,buffer(0,1))
        subtree:add_le(rf24network_proto.fields.from,buffer(1,2))
        subtree:add_le(rf24network_proto.fields.to,buffer(3,2))
        subtree:add_le(rf24network_proto.fields.hID,buffer(5,2))
        subtree:add(rf24network_proto.fields.type,buffer(7,1))
        subtree:add(rf24network_proto.fields.reserved,buffer(8,1))
        subtree:add_le(rf24network_proto.fields.pSize,buffer(9,2))
    end    
    subtree:add_le(buffer(11,pSize),"Data(HEX) " .. buffer(11,pSize))
    
    local subtree = tree:add_le(rf24network_proto,buffer(),"Decode Payload (First Bytes Only)")
    if buffer:len() >= 13 then
        subtree:add_le(rf24network_proto.fields.uint16_t,buffer(11,2))
        subtree:add_le(rf24network_proto.fields.int16_t,buffer(11,2))
    end
    if buffer:len() >= 15 then
        subtree:add_le(rf24network_proto.fields.uint32_t,buffer(11,4))
        subtree:add_le(rf24network_proto.fields.int32_t,buffer(11,4))
    end
end

udp_table = DissectorTable.get("udp.port")
udp_table:add(32001,rf24network_proto)