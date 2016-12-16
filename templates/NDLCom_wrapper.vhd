---------------------------------------------------------------------------------
--! @file    NDLCom_wrapper.vhd
--! @class   NDLCom_wrapper
--!
--! @brief   NDLCom wrapper
--! @details NDLCom wrapper which handles the communication via NDLCom.\n\n
--!          German Research Center for Artificial Intelligence\n
--!
--! @author Tobias Stark (tobias.stark@dfki.de)
--! @date   22.01.2015
---------------------------------------------------------------------------------
-- Last Commit: 
-- $LastChangedRevision: 4522 $
-- $LastChangedBy: tstark $
-- $LastChangedDate: 2016-04-07 15:58:54 +0200 (Do, 07 Apr 2016) $
---------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.config.all;
use work.NDLCom_config.all;
use work.dfki_pack.all;
use work.representations.all;
use work.devices.all;
use work.register_types.all;
use work.register_pack.all;

entity @name@_NDLCom_wrapper is
    generic ( CLK_FREQ       : integer := 16000000;  --! the system clock (in Hz)
              NUMBER_OF_HS_PORTS : natural := <numExternalInterfaces>;     --! number of highspeed communication ports
              NUMBER_OF_LS_PORTS : natural := <numExternalInterfaces>;     --! number of lowspeed communication ports
              LS_BAUD_RATE   : natural := 921600 );  --! baud rate for the lowspeed communication
    port ( CLK     : in  std_logic;                     --! system clock
           RST     : in  std_logic;                     --! system reset
           NODE_ID : in  std_logic_vector(7 downto 0) := std_logic_vector(to_unsigned(<myId>,8));  --! node id
           
           -- Communication pins for highspeed communication
           hs_rx_p : in  std_logic_vector(NUMBER_OF_HS_PORTS-1 downto 0) := (others => '0');  --! LVDS positive RX input for highspeed communication
           hs_rx_n : in  std_logic_vector(NUMBER_OF_HS_PORTS-1 downto 0) := (others => '0');  --! LVDS negative RX input for highspeed communication
           hs_tx_p : out std_logic_vector(NUMBER_OF_HS_PORTS-1 downto 0);  --! LVDS positive TX output for highspeed communication
           hs_tx_n : out std_logic_vector(NUMBER_OF_HS_PORTS-1 downto 0);  --! LVDS negative RX output for highspeed communication
           hs_link_valid    : out std_logic_vector(NUMBER_OF_HS_PORTS-1 downto 0);

           -- Communication pins for lowspeed communication
           ls_rx : in  std_logic_vector(NUMBER_OF_LS_PORTS-1 downto 0) := (others => '0');    --! RX input for lowspeed communication
           ls_tx : out std_logic_vector(NUMBER_OF_LS_PORTS-1 downto 0);    --! TX output for lowspeed communication

           resetDevice      : out std_logic; --! signal for resetting the device (only active for only cycle)
           stopDevice       : out std_logic; --! when this signal is emitted the device should stop moving (active until RESUME cmd is received)
 
           bg_input_portId  : out std_logic_vector(7 downto 0);  --! id of the input of the behavior graph which gets updated
           bg_input_data    : out std_logic_vector(31 downto 0); --! the data for the input
           bg_input_req     : out std_logic;                     --! this will get high iff a behavior graph update packets has been received and id and data are valid
           bg_input_ack     : in std_logic;                      --! has to be set high for one clock cycle for acknowledging data
           bg_output_recvId : in std_logic_vector(7 downto 0);   --! id of the receiving behavior graph which is about to get an input update
           bg_output_portId : in std_logic_vector(7 downto 0);   --! the id of the receiving behavior graph input getting updated
           bg_output_data   : in std_logic_vector(31 downto 0);  --! the data for the other graph
           bg_output_req    : in std_logic;                      --! This will get high iff our behavior graph has produced new data
           bg_output_ack    : out std_logic;                     --! The sender process in this file will set this high for one clock cycle to acknowledge receival

           reg_busy         : in  std_logic;
           reg_read_id      : out std_logic_vector(15 downto 0); --! address of the registers to be send
           reg_read_type    : in  registertype_t;                --! length of the register to be send
           reg_read_data    : in  std_logic_vector(63 downto 0); --! data of the register to be send
           reg_read         : out std_logic;                     --! read register enable signal
           reg_read_ack     : in  std_logic;                     --! read register acknowledge signal
           reg_write_id     : out std_logic_vector(15 downto 0); --! address of the register to be written to
           reg_write_type   : out registertype_t;                --! length of the register to be written to
           reg_write_data   : out std_logic_vector(63 downto 0); --! data of the register to be written to
           reg_write        : out std_logic;                     --! write register enable signal
           
           isp_cmd_upload   : out std_logic;  --! ISP command trigger (high for one clock cycle)
           isp_cmd_data     : out std_logic;  --! ISP data trigger (high for one clock cycle)
           isp_cmd_download : out std_logic;  --! ISP download trigger (high for one clock cycle)
           isp_cmd_ack      : in  std_logic;  --! ISP ack signal to acknowledge one of the above trigger
           isp_din_addr     : out std_logic_vector(31 downto 0);
           isp_din_len      : out std_logic_vector(31 downto 0);
           isp_din          : out std_logic_vector(7 downto 0);
           isp_din_valid    : out std_logic;
           isp_din_ack      : in  std_logic;
           isp_dout_addr    : in  std_logic_vector(31 downto 0);
           isp_dout         : in  std_logic_vector(7 downto 0);
           isp_dout_valid   : in  std_logic;
           isp_dout_ack     : out std_logic;

           rtc_actual_time  : in  std_logic_vector(63 downto 0);  --! actual time (rtc)
           rtc_set_time     : out std_logic;                      --! set time trigger (rtc)
           rtc_new_time     : out std_logic_vector(63 downto 0);  --! new time (rtc)

           msg_recv         : out std_logic;                      --! flag that a message was received (could be used for communication timeouts)
           commErr          : out std_logic;                      --! communication error flag

           ndlc_clear_stat  : in  std_logic := '0';
           ndlc_last_error  : out std_logic_vector(15 downto 0);
           ndlc_recvEvents  : out std_logic_vector(31 downto 0);
           ndlc_missEvents  : out std_logic_vector(31 downto 0) );
end @name@_NDLCom_wrapper;

architecture Behavioral of @name@_NDLCom_wrapper is

    signal nc_readyToSend      : std_logic;
    signal nc_startSending     : std_logic;
    signal nc_newData          : std_logic;
    signal nc_dataAck          : std_logic;
    signal nc_sendReceiver     : std_logic_vector(7 downto 0);
    signal nc_sendFrameCounter : std_logic_vector(7 downto 0);
    signal nc_sendLength       : std_logic_vector(7 downto 0);
    signal nc_send_wea         : std_logic_vector(0 downto 0);
    signal nc_send_addr        : std_logic_vector(7 downto 0);
    signal nc_send_data        : std_logic_vector(7 downto 0);
    signal nc_recvSender       : std_logic_vector(7 downto 0);
    signal nc_recvFrameCounter : std_logic_vector(7 downto 0);
    signal nc_recvLength       : std_logic_vector(7 downto 0);
    signal nc_recv_addr        : std_logic_vector(7 downto 0);
    signal nc_recv_data        : std_logic_vector(7 downto 0);
    signal nc_error            : std_logic;

    signal frameCounter_addr_in  : std_logic_vector(7 downto 0);
    signal frameCounter_data_in  : std_logic_vector(7 downto 0);
    signal frameCounter_wea      : std_logic_vector(0 downto 0);
    signal frameCounter_addr_out : std_logic_vector(7 downto 0);
    signal frameCounter_data_out : std_logic_vector(7 downto 0);

    type receiveStates is (idle, checkId, waitReceiverProcess);
    signal receiveState : receiveStates;
    signal receiveError : std_logic;

    type sendStates is (idle, writeTimeStamp, waitSenderProcess);
    signal sendState : sendStates;

    signal stopDevice_int : std_logic;

    type commTypes is (common, ping, registerValue, isp, behaviorGraph);
    signal sendType : commTypes;
    signal recvType : commTypes;

    signal common_recv_addr : std_logic_vector(7 downto 0);
    signal common_recvError : std_logic;
    signal common_send_addr : std_logic_vector(7 downto 0);
    signal common_send_data : std_logic_vector(7 downto 0);
    signal common_send_wea  : std_logic_vector(0 downto 0);

    -- REGISTER

    type registerRecvStates is (idle, read_checkLength, read_copyAddr,
                                write_checkLength, write_copyAddr,
                                write_copyType, write_copyValue);
    signal registerRecvState : registerRecvStates;

    signal register_recvReadRequest  : std_logic;
    signal register_recvWriteRequest : std_logic;
    signal register_recvAck   : std_logic;
    signal register_recvError : std_logic;
    signal register_recvId    : std_logic_vector(7 downto 0);
    signal register_recv_addr : std_logic_vector(7 downto 0);
    
    type registerSendStates is (idle, waitAck, writeAddr,
                                writeType, writeData);
    signal registerSendState : registerSendStates;

    signal register_sendRequest   : std_logic;
    signal register_sendAck       : std_logic;
    signal register_startSending  : std_logic;
    signal register_send_addr     : std_logic_vector(7 downto 0);
    signal register_send_data     : std_logic_vector(7 downto 0);
    signal register_send_wea      : std_logic_vector(0 downto 0);

    signal reg_read_id_int    : std_logic_vector(15 downto 0);
    signal reg_write_type_int : registertype_t;
    
    -- PING

    type pingRecvStates is (idle, checkLength, checkMode,
                            copyTime, copyId);
    signal pingRecvState : pingRecvStates;

    signal ping_recvRequest : std_logic;
    signal ping_recvAck   : std_logic;
    signal ping_recvError : std_logic;
    signal ping_recvId    : std_logic_vector(7 downto 0);
    signal ping_recv_addr : std_logic_vector(7 downto 0);

    type pingSendStates is (idle, waitAck, writeMode,
                            writeTime, writePingId);
    signal pingSendState : pingSendStates;

    signal ping_sendRequest   : std_logic;
    signal ping_sendAck       : std_logic;
    signal ping_startSending  : std_logic;
    signal ping_send_addr     : std_logic_vector(7 downto 0);
    signal ping_send_data     : std_logic_vector(7 downto 0);
    signal ping_send_wea      : std_logic_vector(0 downto 0);

    signal pingId        : std_logic_vector(15 downto 0);
    signal sendPingReply : std_logic;

    -- ISP

    type ispRecvStates is (idle,
                           cmd_checkLength, cmd_readCmd, cmd_readAddr,
                           cmd_readLength, cmd_waitAck,
                           data_checkLength, data_readAddr, data_readData,
                           data_waitDataAck, data_waitAck);
    signal ispRecvState : ispRecvStates;

    signal isp_recvCmdRequest  : std_logic;
    signal isp_recvDataRequest : std_logic;
    signal isp_recvAck   : std_logic;
    signal isp_recvError : std_logic;
    signal isp_recvId    : std_logic_vector(7 downto 0);
    signal isp_recv_addr : std_logic_vector(7 downto 0);

    type ispSendStates is (idle, cmd_waitAck, cmd_writeAck, cmd_writeAddr,
                           data_waitAck, data_writeLength, data_writeData);
    signal ispSendState : ispSendStates;

    signal isp_sendCmdRequest  : std_logic;
    signal isp_sendDataRequest : std_logic;
    signal isp_sendAck      : std_logic;
    signal isp_startSending : std_logic;
    signal isp_send_addr    : std_logic_vector(7 downto 0);
    signal isp_send_data    : std_logic_vector(7 downto 0);
    signal isp_send_wea     : std_logic_vector(0 downto 0);

    signal isp_din_valid_int  : std_logic;
    signal sendIspAckRequest  : std_logic;
    signal sendIspDataRequest : std_logic;

	-- Behavior Graph
	type behaviorGraphRecvStates is (idle, checkLength, copyId, copyData, sync);
	signal behaviorGraphRecvState : behaviorGraphRecvStates;
	signal behaviorGraph_recvRequest : std_logic;
	signal behaviorGraph_recvAck     : std_logic;
	signal behaviorGraph_recv_addr   : std_logic_vector(7 downto 0);
	
	type behaviorGraphSendStates is (idle, waitAck, writeId, writeData);
	signal behaviorGraphSendState     : behaviorGraphSendStates;
	signal behaviorgraph_sendRequest  : std_logic;
	signal behaviorgraph_sendAck      : std_logic;
	signal behaviorgraph_startSending : std_logic;
	signal behaviorgraph_send_addr    : std_logic_vector(7 downto 0);
	signal behaviorgraph_send_data    : std_logic_vector(7 downto 0);
	signal behaviorgraph_send_wea     : std_logic_vector(0 downto 0);
	
	signal bg_output_recvId_int : std_logic_vector(7 downto 0);
	signal bg_output_portId_int : std_logic_vector(7 downto 0);
	signal bg_output_data_int : std_logic_vector(31 downto 0);

begin

    commErr <= nc_error or receiveError;
    
    stopDevice    <= stopDevice_int;

    reg_read_id    <= reg_read_id_int;
    reg_write_type <= reg_write_type_int;

    isp_din_valid <= isp_din_valid_int and not isp_din_ack;
    
    -- frame counter buffer (simple dual-port ram 256x8) --
    frameCounter_buffer : entity work.bram_dp_simple
        generic map ( ADDRWIDTH => 8,
                      DATAWIDTH => 8 )
        port map ( clk   => CLK,
                   we    => frameCounter_wea(0),
                   waddr => frameCounter_addr_in,
                   wdata => frameCounter_data_in,
                   raddr => frameCounter_addr_out,
                   rdata => frameCounter_data_out);

    -- NodeCommunication module
    NDLCom : entity work.NDLCom_top
        generic map ( CLK_FREQ           => CLK_FREQ,
                      NUMBER_OF_HS_PORTS => NUMBER_OF_HS_PORTS,
                      NUMBER_OF_LS_PORTS => NUMBER_OF_LS_PORTS,
                      LS_BAUD_RATE       => LS_BAUD_RATE )
        port map ( CLK => CLK,
                   RST => RST,
                   NODE_ID => NODE_ID,
                   
                   hs_rx_p => hs_rx_p,
                   hs_rx_n => hs_rx_n,
                   hs_tx_p => hs_tx_p,
                   hs_tx_n => hs_tx_n,

                   ls_rx   => ls_rx,
                   ls_tx   => ls_tx,
                   
                   readyToSend      => nc_readyToSend,
                   startSending     => nc_startSending,
                   sendReceiver     => nc_sendReceiver,
                   sendFrameCounter => nc_sendFrameCounter,
                   sendLength       => nc_sendLength,
                   send_wea         => nc_send_wea,
                   send_addr        => nc_send_addr,
                   send_data        => nc_send_data,

                   newData          => nc_newData,
                   dataAck          => nc_dataAck,
                   recvSender       => nc_recvSender,
                   recvFrameCounter => nc_recvFrameCounter,
                   recvLength       => nc_recvLength,
                   recv_addr        => nc_recv_addr,
                   recv_data        => nc_recv_data,

                   error            => nc_error,

                   -- debug / statistics
                   last_error       => ndlc_last_error,
                   clear_last_error => ndlc_clear_stat,
                   recvEvents       => ndlc_recvEvents,
                   missEvents       => ndlc_missEvents,
                   clear_recvEvents => ndlc_clear_stat );

    
    -- ----------------------------------------------------------------------------------------------
    --                                        RECEIVING
    -- ----------------------------------------------------------------------------------------------
    
    nc_recv_addr       <= ping_recv_addr     when recvType=ping else
                          isp_recv_addr      when recvType=isp else
                          register_recv_addr when recvType=registerValue else
						  behaviorgraph_recv_addr when recvType=behaviorGraph else
                          common_recv_addr;

    receiveError <= common_recvError or ping_recvError or register_recvError or isp_recvError;
    
    receiver : process (CLK)
        variable idleFlag  : std_logic;
    begin
        if CLK='1' and CLK'event then
            if RST='1' then
                idleFlag := '0';

                resetDevice <= '0';
                stopDevice_int <= '0';
                
                nc_dataAck  <= '0';
                common_recv_addr <= (others => '0');
                recvType    <= common;

                register_recvReadRequest  <= '0';
                register_recvWriteRequest <= '0';
                ping_recvRequest          <= '0';
                isp_recvCmdRequest        <= '0';
                isp_recvDataRequest       <= '0';
				behaviorgraph_recvRequest <= '0';

                msg_recv         <= '0';
                common_recvError <= '0';
                receiveState     <= idle;

            else
                -- defaults
                nc_dataAck       <= '0';
                msg_recv         <= '0';
                common_recvError <= '0';

                if idleFlag='1' then
                    idleFlag := '0';
                else

                    case receiveState is
                        
                        when idle =>
                            -- wait for new data
                            if nc_newData='1' then
                                idleFlag         := '1';
                                common_recv_addr <= (others => '0');
                                receiveState     <= checkId;
                            else
                                receiveState     <= idle;
                            end if;

                        when checkId =>

                            -- check message Id
                            case nc_recv_data is
                                
                                when Representation_Id_RESET =>
                                    resetDevice  <= '1';
                                    nc_dataAck   <= '1';
                                    msg_recv     <= '1';
                                    receiveState <= idle;

                                when Representation_Id_STOP =>
                                    stopDevice_int <= '1';
                                    nc_dataAck     <= '1'; 
                                    msg_recv       <= '1';
                                    receiveState   <= idle;

                                when Representation_Id_RESUME =>
                                    stopDevice_int <= '0';
                                    nc_dataAck     <= '1';
                                    msg_recv       <= '1';
                                    receiveState   <= idle;

                                when Representation_Id_RepresentationsPing =>
                                    recvType         <= ping;
                                    ping_recvRequest <= '1';
                                    receiveState     <= waitReceiverProcess;
                                    
                                when Representation_Id_IspCommand =>
                                    recvType           <= isp;
                                    isp_recvCmdRequest <= '1';
                                    receiveState       <= waitReceiverProcess;

                                when Representation_Id_IspData =>
                                    recvType            <= isp;
                                    isp_recvDataRequest <= '1';
                                    receiveState        <= waitReceiverProcess;

                                when Representation_Id_RegisterValueQuery =>
                                    recvType                 <= registerValue;
                                    register_recvReadRequest <= '1';
                                    receiveState             <= waitReceiverProcess;

                                when Representation_Id_RegisterValueWrite =>
                                    recvType                  <= registerValue;
                                    register_recvWriteRequest <= '1';
                                    receiveState              <= waitReceiverProcess;

                                when Representation_Id_BGData =>
                                    recvType                  <= behaviorGraph;
                                    behaviorGraph_recvRequest <= '1';
                                    receiveState              <= waitReceiverProcess;
                                    
                                when others =>
                                    nc_dataAck       <= '1';
                                    common_recvError <= '1';
                                    receiveState     <= idle;
                                    
                            end case;

                        when waitReceiverProcess =>
                            if (recvType=registerValue and register_recvAck='1') or
                                (recvType=ping and ping_recvAck='1') or
								(recvType=behaviorGraph and behaviorGraph_recvAck='1') or
                                (recvType=isp and isp_recvAck='1') then
                                recvType <= common;
                                register_recvReadRequest  <= '0';
                                register_recvWriteRequest <= '0';
                                ping_recvRequest          <= '0';
                                isp_recvCmdRequest        <= '0';
                                isp_recvDataRequest       <= '0';
								behaviorGraph_recvRequest <= '0';
                                msg_recv     <= '1';
                                nc_dataAck   <= '1';
                                receiveState <= idle;
                            end if;
                            
                    end case;
                end if;
            end if;
        end if;
    end process receiver;

    -- ----------------------------------------------------------------------------------------------
    --                                       SENDING
    -- ----------------------------------------------------------------------------------------------

    nc_sendFrameCounter <= frameCounter_data_out;
    nc_send_addr <= ping_send_addr      when sendType=ping else
                    isp_send_addr       when sendType=isp else
                    register_send_addr  when sendType=registerValue else
					behaviorgraph_send_addr when sendType=behaviorgraph else
                    common_send_addr;
    nc_send_data <= ping_send_data      when sendType=ping else
                    isp_send_data       when sendType=isp else
                    register_send_data  when sendType=registerValue else
					behaviorgraph_send_data when sendType=behaviorgraph else
                    common_send_data;
    nc_send_wea <= ping_send_wea      when sendType=ping else
                   isp_send_wea       when sendType=isp else
                   register_send_wea  when sendType=registerValue else
				   behaviorgraph_send_wea when sendType=behaviorgraph else
                   common_send_wea;
    
    sender : process (CLK)
        variable counter    : integer range 0 to 7;
        variable nextSendType  : commTypes;
        variable sendType_1 : commTypes;
        variable sendType_2 : commTypes;
    begin
        if CLK='1' and CLK'event then
            if RST='1' then
                counter    := 0;
                nextSendType  := common;
                sendType_1 := common;
                sendType_2 := common;
                sendType   <= common;
                
                frameCounter_addr_in  <= (others => '0');
                frameCounter_data_in  <= (others => '0');
                frameCounter_wea      <= "0";
                frameCounter_addr_out <= (others => '0');

                nc_sendReceiver  <= (others => '0');
                nc_sendLength    <= (others => '0');
                nc_startSending  <= '0';

                common_send_wea  <= "0";
                common_send_addr <= (others => '0');
                common_send_data <= (others => '0');

                register_sendAck  <= '0';
                ping_sendAck      <= '0';
                isp_sendAck       <= '0';
                behaviorgraph_sendAck <= '0';
                
                sendState <= idle;

            else
                -- defaults
                frameCounter_wea  <= "0";
                nc_startSending   <= '0';
                common_send_wea   <= "0";
                ping_sendAck      <= '0';
                isp_sendAck       <= '0';
                register_sendAck  <= '0';
                behaviorgraph_sendAck <= '0';

                -- 2 clock cycles delay for sendType
                sendType   <= sendType_2;
                sendType_2 := sendType_1;
                
                case sendState is

                    when idle =>
                        sendType <= common;
                        counter  := 0;
                        if ping_sendRequest='1' and nc_readyToSend='1' then
                            nextSendType  := ping;
                            frameCounter_addr_in  <= ping_recvId;
                            frameCounter_addr_out <= ping_recvId;
                            -- write header
                            nc_sendReceiver     <= ping_recvId;
                            nc_sendLength       <= x"14";
                            -- write message type
                            common_send_addr <= x"00";
                            common_send_data <= Representation_Id_RepresentationsPing;
                            common_send_wea  <= "1";
                            sendState        <= writeTimeStamp;

                        elsif isp_sendCmdRequest='1' and nc_readyToSend='1' then
                            nextSendType  := isp;
                            frameCounter_addr_in  <= isp_recvId;
                            frameCounter_addr_out <= isp_recvId;
                            -- write header
                            nc_sendReceiver     <= isp_recvId;
                            nc_sendLength       <= x"12";
                            -- write message type
                            common_send_addr <= x"00";
                            common_send_data <= Representation_Id_IspCommand;
                            common_send_wea  <= "1";
                            sendState        <= writeTimeStamp;

                        elsif isp_sendDataRequest='1' and nc_readyToSend='1' then
                            nextSendType  := isp;
                            frameCounter_addr_in  <= isp_recvId;
                            frameCounter_addr_out <= isp_recvId;
                            -- write header
                            nc_sendReceiver     <= isp_recvId;
                            nc_sendLength       <= x"8d";  -- 128+4+1+8
                            -- write message type
                            common_send_addr <= x"00";
                            common_send_data <= Representation_Id_IspData;
                            common_send_wea  <= "1";
                            sendState        <= writeTimeStamp;

                        elsif register_sendRequest='1' and nc_readyToSend='1' then
                            nextSendType  := registerValue;
                            frameCounter_addr_in  <= register_recvId;
                            frameCounter_addr_out <= register_recvId;
                            -- write header
                            nc_sendReceiver     <= register_recvId;
                            nc_sendLength       <= x"14";
                            -- write message type
                            common_send_addr <= x"00";
                            common_send_data <= Representation_Id_RegisterValueResponse;
                            common_send_wea  <= "1";
                            sendState        <= writeTimeStamp;

                        elsif behaviorgraph_sendRequest='1' and nc_readyToSend='1' then
                             nextSendType := behaviorgraph;
                             frameCounter_addr_in  <= bg_output_recvId_int;
                             frameCounter_addr_out <= bg_output_recvId_int;
                             -- write header
                             nc_sendReceiver		  <= bg_output_recvId_int;
                             nc_sendLength         <= x"0E";
                             -- write message type
                             common_send_addr      <= x"00";
                             common_send_data		  <= Representation_Id_BGData;
                             common_send_wea       <= "1";
                             sendState             <= writeTimeStamp;

                        else
                            sendState <= idle;
                        end if;

                    when writeTimeStamp =>
                        common_send_addr <= incr_f(common_send_addr);
                        common_send_data <= rtc_actual_time(counter*8+7 downto counter*8);
                        common_send_wea  <= "1";
                        if counter=1 then
                            -- increment frame counter
                            frameCounter_data_in <= incr_f(frameCounter_data_out);
                            frameCounter_wea     <= "1";
                        end if;
                        if counter < 7 then
                            counter := counter + 1;
                            sendState <= writeTimeStamp;
                        else
                            sendType_1 := nextSendType;
                            sendState <= waitSenderProcess;
                            case nextSendType is
                                when ping          => ping_sendAck <= '1';
                                when isp           => isp_sendAck <= '1';
                                when registerValue => register_sendAck <= '1';
                                when behaviorgraph => behaviorgraph_sendAck <= '1';
                                when others => sendState <= idle;
                            end case;
                        end if;

                    when waitSenderProcess =>
                        if (sendType=ping and ping_startSending='1') or
                           (sendType=isp and isp_startSending='1') or
                           (sendType=registerValue and register_startSending='1') or
                           (sendType=behaviorgraph and behaviorgraph_startSending='1') then
                            nc_startSending <= '1';
                            sendType_1 := common;
                            sendType_2 := common;
                            sendType   <= common;
                            sendState  <= idle;
                        end if;

                end case;
            end if;
        end if;
    end process sender;

    -- --------------------------------------------------------------------------
    --                             REGISTER
    -- --------------------------------------------------------------------------

    register_receiver : process (CLK)
        variable idleFlag    : std_logic;
        variable byteCounter : integer range 0 to 7;
    begin
        if CLK'event and CLK='1' then
            if RST='1' then
                idleFlag    := '0';
                byteCounter := 0;

                register_recv_addr <= (others => '0');
                register_recvAck   <= '0';
                register_recvError <= '0';
                register_recvId    <= (others => '0');

                reg_read           <= '0';
                reg_read_id_int    <= (others => '0');
                reg_write          <= '0';
                reg_write_id       <= (others => '0');
                reg_write_type_int <= ERROR;
                reg_write_data     <= (others => '0');
                
                registerRecvState <= idle;
            else
                -- defaults
                register_recvAck   <= '0';
                register_recvError <= '0';
                reg_read  <= '0';
                reg_write <= '0';

                if idleFlag='1' then
                    idleFlag := '0';
                else
                    
                    case registerRecvState is

                        when idle =>
                            if register_recvReadRequest='1' then
                                registerRecvState <= read_checkLength;
                            elsif register_recvWriteRequest='1' then
                                registerRecvState <= write_checkLength;
                            end if;
                            
                            -- register read
                            -- -------------
                        when read_checkLength =>
                            -- first check if the register_access
                            -- entity is not busy
                            if reg_busy='0' then
                                idleFlag := '1';
                                -- check if data length is 11 bytes
                                -- (id(1) + timestamp(8) + reg_addr(2))
                                if nc_recvLength=x"0b" then
                                    byteCounter := 0;
                                    register_recvId    <= nc_recvSender;
                                    register_recv_addr <= x"09";
                                    registerRecvState  <= read_copyAddr;
                                else
                                    register_recvAck   <= '1';
                                    register_recvError <= '1';
                                    registerRecvState  <= idle;
                                end if;
                            end if;
                            
                        when read_copyAddr =>
                            idleFlag := '1';
                            if byteCounter=0 then
                                byteCounter := 1;
                                reg_read_id_int(7 downto 0) <= nc_recv_data;
                                register_recv_addr <= x"0a";
                                registerRecvState  <= read_copyAddr;
                            else
                                reg_read_id_int(15 downto 8) <= nc_recv_data;
                                reg_read          <= '1';
                                register_recvAck  <= '1';
                                registerRecvState <= idle;
                            end if;

                            -- register write
                            -- --------------
                        when write_checkLength =>
                            -- first check if the register_access
                            -- entity is not busy
                            if reg_busy='0' then
                                idleFlag := '1';
                                -- check if data length is 20 bytes
                                -- (id(1) + timestamp(8) + reg_addr(2) + type(1) + value(8))
                                if nc_recvLength=x"14" then
                                    byteCounter := 0;
                                    register_recvId    <= nc_recvSender;
                                    register_recv_addr <= x"09";
                                    registerRecvState  <= write_copyAddr;
                                else
                                    register_recvAck   <= '1';
                                    register_recvError <= '1';
                                    registerRecvState  <= idle;
                                end if;
                            end if;

                        when write_copyAddr =>
                            idleFlag := '1';
                            if byteCounter=0 then
                                byteCounter := 1;
                                reg_write_id(7 downto 0) <= nc_recv_data;
                                register_recv_addr <= x"0a";
                                registerRecvState  <= write_copyAddr;
                            else
                                reg_write_id(15 downto 8) <= nc_recv_data;
                                register_recv_addr <= x"0b";
                                registerRecvState  <= write_copyType;
                            end if;

                        when write_copyType =>
                            idleFlag    := '1';
                            byteCounter := 0;
                            reg_write_type_int <= enum2registertype(nc_recv_data);
                            register_recv_addr <= x"0c";
                            registerRecvState  <= write_copyValue;

                        when write_copyValue =>
                            idleFlag    := '1';
                            reg_write_data(byteCounter*8+7 downto byteCounter*8) <= nc_recv_data;
                            if byteCounter < registertype2len(reg_write_type_int)-1 then
                                byteCounter := byteCounter + 1;
                                register_recv_addr <= incr_f(register_recv_addr);
                                registerRecvState  <= write_copyValue;
                            else
                                reg_write <= '1';
                                register_recvAck  <= '1';
                                registerRecvState <= idle;
                            end if;

                    end case;
                end if;
            end if;
        end if;
    end process;
    
    register_sender : process (CLK)
        variable byteCounter : integer range 0 to 8;
    begin
        if CLK'event and CLK='1' then
            if RST='1' then
                byteCounter := 0;
                
                register_sendRequest  <= '0';
                register_startSending <= '0';
                
                register_send_addr <= (others => '0');
                register_send_data <= (others => '0');
                register_send_wea  <= "0";

                registerSendState <= idle;
            else
                -- defaults
                register_startSending <= '0';
                register_send_wea <= "0";

                case registerSendState is

                    when idle =>
                        if reg_read_ack='1' then
                            register_sendRequest <= '1';
                            registerSendState    <= waitAck;
                        end if;

                    when waitAck =>
                        if register_sendAck='1' then
                            byteCounter := 0;
                            register_sendRequest <= '0';
                            registerSendState    <= writeAddr;
                        else
                            registerSendState    <= waitAck;
                        end if;
                        
                    when writeAddr =>
                        if byteCounter=0 then
                            byteCounter := 1;
                            register_send_addr <= x"09";
                            register_send_data <= reg_read_id_int(7 downto 0);
                            register_send_wea  <= "1";
                            registerSendState  <= writeAddr;
                        else
                            register_send_addr <= x"0a";
                            register_send_data <= reg_read_id_int(15 downto 8);
                            register_send_wea  <= "1";
                            registerSendState  <= writeType;
                        end if;

                    when writeType =>
                        byteCounter := 0;
                        register_send_addr <= x"0b";
                        register_send_data <= registertype2enum(reg_read_type);
                        register_send_wea  <= "1";
                        registerSendState  <= writeData;

                    when writeData =>
                        if byteCounter < registertype2len(reg_read_type) then
                            register_send_addr <= std_logic_vector(x"0c" + to_unsigned(byteCounter,8));
                            register_send_data <= reg_read_data(byteCounter*8+7 downto byteCounter*8);
                            register_send_wea  <= "1";
                            byteCounter := byteCounter + 1;
                            registerSendState  <= writeData;
                        else
                            register_startSending <= '1';
                            registerSendState  <= idle;
                        end if;

                end case;
            end if;
        end if;
    end process;

    -- --------------------------------------------------------------------------
    --                                PING
    -- --------------------------------------------------------------------------

    ping_receiver : process (CLK)
        variable idleFlag    : std_logic;
        variable byteCounter : integer range 0 to 7;
    begin
        if CLK'event and CLK='1' then
            if RST='1' then
                idleFlag    := '0';
                byteCounter := 0;

                ping_recv_addr <= (others => '0');
                ping_recvAck   <= '0';
                ping_recvError <= '0';
                ping_recvId    <= (others => '0');

                rtc_new_time  <= (others => '0');
                rtc_set_time  <= '0';
                sendPingReply <= '0';

                pingRecvState <= idle;
            else
                -- defaults
                ping_recvAck   <= '0';
                ping_recvError <= '0';
                rtc_set_time   <= '0';
                sendPingReply  <= '0';

                if idleFlag='1' then
                    idleFlag := '0';
                else
                    
                    case pingRecvState is

                        when idle =>
                            if ping_recvRequest='1' then
                                pingRecvState <= checkLength;
                            end if;
                            
                        when checkLength =>
                            -- check if data length is 20 bytes (id + timestamp +
                            -- mode + 8byte time value + 2byte ping id)
                            if nc_recvLength=x"14" then
                                idleFlag   := '1';
                                ping_recvId    <= nc_recvSender;
                                ping_recv_addr <= x"09";
                                pingRecvState  <= checkMode;
                            else
                                idleFlag := '1';
                                ping_recvAck   <= '1';
                                ping_recvError <= '1';
                                pingRecvState  <= idle;
                            end if;
                            
                        when checkMode =>
                            if nc_recv_data=x"01" then
                                -- ping request
                                idleFlag    := '1';
                                byteCounter := 0;
                                ping_recv_addr <= x"12";
                                pingRecvState  <= copyId;
                            elsif nc_recv_data=x"04" then
                                -- ping set time
                                idleFlag    := '1';
                                byteCounter := 0;
                                ping_recv_addr <= x"0a";
                                pingRecvState  <= copyTime;
                            else
                                idleFlag := '1';
                                ping_recvAck   <= '1';
                                ping_recvError <= '1';
                                pingRecvState  <= idle;
                            end if;

                        when copyTime =>
                            rtc_new_time(byteCounter*8+7 downto byteCounter*8) <= nc_recv_data;
                            if byteCounter < 7 then
                                idleFlag    := '1';
                                byteCounter := byteCounter + 1;
                                ping_recv_addr <= incr_f(ping_recv_addr);
                                pingRecvState  <= copyTime;
                            else
                                idleFlag := '1';
                                rtc_set_time  <= '1';
                                ping_recvAck  <= '1';
                                pingRecvState <= idle;
                            end if;

                        when copyId =>
                            pingId(byteCounter*8+7 downto byteCounter*8) <= nc_recv_data;
                            if byteCounter < 1 then
                                idleFlag    := '1';
                                byteCounter := byteCounter + 1;
                                ping_recv_addr <= incr_f(ping_recv_addr);
                                pingRecvState  <= copyId;
                            else
                                idleFlag := '1';
                                sendPingReply <= '1';
                                ping_recvAck  <= '1';
                                pingRecvState <= idle;
                            end if;

                    end case;
                end if;
            end if;
        end if;
    end process;
    
    ping_sender : process (CLK)
        variable dataCounter : integer range 0 to 7;
    begin
        if CLK'event and CLK='1' then
            if RST='1' then
                dataCounter := 0;

                ping_sendRequest  <= '0';
                ping_startSending <= '0';

                ping_send_addr <= (others => '0');
                ping_send_data <= (others => '0');
                ping_send_wea  <= "0";

                pingSendState <= idle;
            else
                -- defaults
                ping_startSending <= '0';
                ping_send_wea <= "0";

                case pingSendState is

                    when idle =>
                        if sendPingReply='1' then
                            ping_sendRequest <= '1';
                            pingSendState    <= waitAck;
                        end if;

                    when waitAck =>
                        if ping_sendAck='1' then
                            ping_sendRequest <= '0';
                            pingSendState    <= writeMode;
                        else
                            pingSendState    <= waitAck;
                        end if;
                        
                    when writeMode =>
                        ping_send_addr <= x"09";
                        ping_send_data <= x"02";
                        ping_send_wea  <= "1";
                        dataCounter    := 0;
                        pingSendState  <= writeTime;

                    when writeTime =>
                        ping_send_addr <= incr_f(ping_send_addr);
                        ping_send_data <= rtc_actual_time(dataCounter*8+7 downto dataCounter*8);
                        ping_send_wea  <= "1";
                        if dataCounter < 7 then
                            dataCounter   := dataCounter + 1;
                            pingSendState <= writeTime;
                        else
                            dataCounter   := 0;
                            pingSendState <= writePingId;
                        end if;

                    when writePingId =>
                        ping_send_addr <= incr_f(ping_send_addr);
                        ping_send_data <= pingId(dataCounter*8+7 downto dataCounter*8);
                        ping_send_wea  <= "1";
                        if dataCounter < 1 then
                            dataCounter   := dataCounter + 1;
                            pingSendState <= writePingId;
                        else
                            ping_startSending <= '1';
                            pingSendState <= idle;
                        end if;

                end case;
            end if;
        end if;
    end process;

    -- --------------------------------------------------------------------------
    --                                ISP
    -- --------------------------------------------------------------------------

    isp_receiver : process (CLK)
        variable idleFlag    : std_logic;
        variable byteCounter : integer range 0 to 127;
        variable isp_cmd_int : std_logic_vector(2 downto 0);
    begin
        if CLK'event and CLK='1' then
            if RST='1' then
                idleFlag    := '0';
                byteCounter := 0;
                isp_cmd_int := (others => '0');

                isp_recv_addr <= (others => '0');
                isp_recvAck   <= '0';
                isp_recvError <= '0';
                isp_recvId    <= (others => '0');

                isp_cmd_upload   <= '0';
                isp_cmd_download <= '0';
                isp_cmd_data <= '0';
                isp_din_addr <= (others => '0');
                isp_din_len  <= (others => '0');
                isp_din      <= (others => '0');
                isp_din_valid_int <= '0';

                sendIspAckRequest  <= '0';
                sendIspDataRequest <= '0';

                ispRecvState <= idle;
            else
                -- defaults
                isp_recvAck       <= '0';
                isp_recvError     <= '0';
                isp_cmd_upload    <= '0';
                isp_cmd_data      <= '0';
                isp_cmd_download  <= '0';
                isp_din_valid_int <= '0';
                sendIspAckRequest  <= '0';
                sendIspDataRequest <= '0';

                if idleFlag='1' then
                    idleFlag := '0';
                else

                    case ispRecvState is

                        when idle =>
                            if isp_recvCmdRequest='1' then
                                ispRecvState <= cmd_checkLength;
                            elsif isp_recvDataRequest='1' then
                                ispRecvState  <= data_checkLength;
                            end if;
                            
                        when cmd_checkLength =>
                            -- check if data length is 18 bytes (id + timestamp + cmd + addr + len)
                            if nc_recvLength=x"12" then
                                idleFlag := '1';
                                isp_recvId    <= nc_recvSender;
                                isp_recv_addr <= x"09";
                                ispRecvState  <= cmd_readCmd;
                            else
                                idleFlag := '1';
                                isp_recvAck   <= '1';
                                isp_recvError <= '1';
                                ispRecvState  <= idle;
                            end if;

                        when cmd_readCmd =>
                            idleFlag := '1';
                            byteCounter   := 0;
                            isp_cmd_int   := nc_recv_data(2 downto 0);
                            isp_recv_addr <= incr_f(isp_recv_addr);
                            ispRecvState  <= cmd_readAddr;

                        when cmd_readAddr =>
                            idleFlag := '1';
                            isp_din_addr(byteCounter*8+7 downto byteCounter*8) <= nc_recv_data;
                            isp_recv_addr <= incr_f(isp_recv_addr);
                            if byteCounter < 3 then
                                byteCounter  := byteCounter + 1;
                                ispRecvState <= cmd_readAddr;
                            else
                                byteCounter  := 0;
                                ispRecvState <= cmd_readLength;
                            end if;

                        when cmd_readLength =>
                            idleFlag := '1';
                            isp_din_len(byteCounter*8+7 downto byteCounter*8) <= nc_recv_data;
                            isp_recv_addr <= incr_f(isp_recv_addr);
                            if byteCounter < 3 then
                                byteCounter  := byteCounter + 1;
                                ispRecvState <= cmd_readLength;
                            else
                                idleFlag := '1';
                                byteCounter  := 0;
                                isp_recvAck  <= '1';
                                ispRecvState <= idle;
                                if isp_cmd_int="000" then  -- upload
                                    isp_cmd_upload <= '1';
                                    ispRecvState   <= cmd_waitAck;
                                elsif isp_cmd_int="001" then  -- download
                                    isp_cmd_download   <= '1';
                                    sendIspDataRequest <= '1';
                                end if;
                            end if;

                        when cmd_waitAck =>
                            if isp_cmd_ack='1' then
                                if isp_cmd_int="000" then  -- upload
                                    sendIspAckRequest <= '1';
                                end if;
                                ispRecvState <= idle;
                            else
                                ispRecvState <= cmd_waitAck;
                            end if;

                        when data_checkLength =>
                            -- check if data length is 133 bytes (id + timestamp + addr + 128bytes)
                            if nc_recvLength=x"8d" then
                                idleFlag := '1';
                                isp_recvId    <= nc_recvSender;
                                isp_recv_addr <= x"09";
                                ispRecvState  <= data_readAddr;
                            else
                                idleFlag := '1';
                                isp_recvAck   <= '1';
                                isp_recvError <= '1';
                                ispRecvState  <= idle;
                            end if;

                        when data_readAddr =>
                            idleFlag := '1';
                            isp_cmd_data <= '1';
                            isp_din_addr(byteCounter*8+7 downto byteCounter*8) <= nc_recv_data;
                            isp_recv_addr <= incr_f(isp_recv_addr);
                            if byteCounter < 3 then
                                byteCounter  := byteCounter + 1;
                                ispRecvState <= data_readAddr;
                            else
                                byteCounter  := 0;
                                ispRecvState <= data_readData;
                            end if;

                        when data_readData =>
                            isp_din <= nc_recv_data;
                            isp_din_valid_int <= '1';
                            if byteCounter < 127 then
                                byteCounter   := byteCounter + 1;
                                isp_recv_addr <= incr_f(isp_recv_addr);
                                ispRecvState  <= data_waitDataAck;
                            else
                                idleFlag := '1';
                                byteCounter  := 0;
                                isp_recvAck  <= '1';
                                ispRecvState <= data_waitAck;
                            end if;

                        when data_waitDataAck =>
                            if isp_din_ack='1' then
                                ispRecvState <= data_readData;
                            else
                                isp_din_valid_int <= '1';
                                ispRecvState <= data_waitDataAck;
                            end if;

                        when data_waitAck =>
                            if isp_cmd_ack='1' then
                                sendIspAckRequest <= '1';
                                ispRecvState <= idle;
                            else
                                ispRecvState <= data_waitAck;
                            end if;
                    end case;
                end if;
            end if;
        end if;
    end process;
    
    isp_sender : process (CLK)
        variable byteCounter : integer range 0 to 127;
    begin
        if CLK'event and CLK='1' then
            if RST='1' then
                byteCounter := 0;
                
                isp_sendCmdRequest  <= '0';
                isp_sendDataRequest <= '0';
                isp_startSending    <= '0';

                isp_send_addr <= (others => '0');
                isp_send_data <= (others => '0');
                isp_send_wea  <= "0";
                isp_dout_ack  <= '0';
                
                ispSendState <= idle;
            else
                -- defaults
                isp_startSending <= '0';
                isp_send_wea <= "0";
                isp_dout_ack <= '0';

                case ispSendState is

                    when idle =>
                        if sendIspAckRequest='1' then
                            isp_sendCmdRequest <= '1';
                            isp_send_addr <= x"08";
                            ispSendState  <= cmd_waitAck;
                        elsif sendIspDataRequest='1' then
                            isp_sendDataRequest <= '1';
                            isp_send_addr <= x"08";
                            ispSendState  <= data_waitAck;
                        end if;

                    when cmd_waitAck =>
                        if isp_sendAck='1' then
                            isp_sendCmdRequest <= '0';
                            ispSendState <= cmd_writeAck;
                        else
                            ispSendState <= cmd_waitAck;
                        end if;

                    when cmd_writeAck =>
                        isp_send_addr <= incr_f(isp_send_addr);
                        isp_send_data <= x"03";
                        isp_send_wea  <= "1";
                        ispSendState  <= cmd_writeAddr;

                    when cmd_writeAddr =>
                        isp_send_addr <= incr_f(isp_send_addr);
                        isp_send_data <= isp_dout_addr(byteCounter*8+7 downto byteCounter*8);
                        isp_send_wea  <= "1";
                        if byteCounter < 3 then
                            byteCounter  := byteCounter + 1;
                            ispSendState <= cmd_writeAddr;
                        else
                            byteCounter  := 0;
                            isp_startSending <= '1';
                            ispSendState <= idle;
                        end if;

                    when data_waitAck =>
                        if isp_sendAck='1' then
                            isp_sendDataRequest <= '0';
                            ispSendState <= data_writeLength;
                        else
                            ispSendState <= data_waitAck;
                        end if;
                        
                    when data_writeLength =>
                        isp_send_addr <= incr_f(isp_send_addr);
                        isp_send_data <= isp_dout_addr(byteCounter*8+7 downto byteCounter*8);
                        isp_send_wea  <= "1";
                        if byteCounter < 3 then
                            byteCounter  := byteCounter + 1;
                            ispSendState <= data_writeLength;
                        else
                            byteCounter  := 0;
                            ispSendState <= data_writeData;
                        end if;

                    when data_writeData =>
                        if isp_dout_valid='1' then
                            isp_send_addr <= incr_f(isp_send_addr);
                            isp_send_data <= isp_dout;
                            isp_send_wea  <= "1";
                            isp_dout_ack  <= '1';
                            if byteCounter < 127 then
                                byteCounter  := byteCounter + 1;
                                ispSendState <= data_writeData;
                            else
                                byteCounter  := 0;
                                isp_startSending <= '1';
                                ispSendState <= idle;
                            end if;
                        end if;

                end case;
            end if;
        end if;
    end process;

    -- --------------------------------------------------------------------------
    --                                Behavior Graph
    -- --------------------------------------------------------------------------

	 behaviorGraph_receiver : process(clk)
	     variable idleFlag : std_logic;
		  variable byteCounter : integer range 0 to 3;
	 begin
	     if CLK'event and CLK='1' then
            if RST='1' then
                bg_input_portid <= (others => '0');
				bg_input_data   <= (others => '0');
				bg_input_req    <= '0';
					
				idleFlag := '0';
				byteCounter := 0;
					
				behaviorGraph_recvAck <= '0';
				behaviorGraphRecvState <= idle;
			else
			   -- defaults
			   behaviorGraph_recvAck <= '0';
               bg_input_req <= '0';
					
               if idleFlag = '1' then
                   idleFlag := '0';
               else
                   case behaviorGraphRecvState is
                        when idle =>
                            if (behaviorGraph_recvRequest = '1') then
                                behaviorGraphRecvState <= checkLength;
                            else
                                behaviorGraphRecvState <= idle;
                            end if;
                            
                        when checkLength =>
                            idleFlag := '1';
                            -- check if data length is 15 bytes (id + timestamp +
                            -- 1byte output + 1byte input + 4byte float data)
                             if nc_recvLength=x"0E" then
                                  behaviorGraph_recv_addr <= x"09";
                                  behaviorGraphRecvState  <= copyId;
                             else
                                  behaviorGraph_recvAck   <= '1';
                                  behaviorGraphRecvState  <= idle;
                             end if;
                             
                        when copyId =>
                            -- Set the id of the input
                           idleFlag := '1';
                            byteCounter := 0;
                            bg_input_portid <= nc_recv_data;
                            behaviorGraph_recv_addr <= incr_f(behaviorGraph_recv_addr);
                            behaviorGraphRecvState <= copyData;
                            
                        when copyData =>
                            idleFlag := '1';
                            -- Set the received data
                           bg_input_data(byteCounter*8+7 downto byteCounter*8) <= nc_recv_data;
                            if (byteCounter < 3) then
                                byteCounter := byteCounter + 1;
                                behaviorGraph_recv_addr <= incr_f(behaviorGraph_recv_addr);
                                behaviorGraphRecvState <= copyData;
                            else
                               behaviorGraph_recvAck <= '1';
                                behaviorGraphRecvState <= sync;
                            end if;
                            
                        when sync =>
                           -- Wait until the behavior graph has seen this
                            bg_input_req <= '1';
                            if (bg_input_ack = '1') then
                                bg_input_req <= '0';
                                behaviorGraphRecvState <= idle;
                            else
                                behaviorGraphRecvState <= sync;
                            end if;
                    end case;
                end if;
            end if;
		  end if;
	 end process;
	 
	 behaviorgraph_sender : process(clk)
		variable byteCounter : integer range 0 to 3;
	 begin
		if CLK'event and CLK='1' then
			if RST='1' then
				byteCounter := 0;
				bg_output_recvId_int <= (others => '0');
				bg_output_portId_int <= (others => '0');
				bg_output_data_int <= (others => '0');
				bg_output_ack <= '0';
				
				behaviorgraph_sendRequest <= '0';
				behaviorgraph_startSending <= '0';
				behaviorgraph_send_addr <= (others => '0');
				behaviorgraph_send_data <= (others => '0');
				behaviorgraph_send_wea <= "0";
				
				behaviorGraphSendState <= idle;
			else
				-- defaults
				bg_output_ack <= '0';
				behaviorgraph_startSending <= '0';
				behaviorgraph_send_wea     <= "0";
				
				case behaviorGraphSendState is
					when idle =>
						if bg_output_req = '1' then
						   bg_output_recvId_int <= bg_output_recvId;
							bg_output_portId_int <= bg_output_portid;
							bg_output_data_int <= bg_output_data;
							bg_output_ack <= '1';
							behaviorgraph_sendRequest <= '1';
							behaviorGraphSendState <= waitAck;
						else
							behaviorGraphSendState <= idle;
						end if;
						
					when waitAck =>
						if behaviorgraph_sendAck = '1' then
							behaviorgraph_sendRequest <= '0';
							behaviorGraphSendState <= writeId;
						else
							behaviorGraphSendState <= waitAck;
						end if;
						
					when writeId =>
						behaviorgraph_send_addr <= x"09";
						behaviorgraph_send_data <= bg_output_portid_int;
						behaviorgraph_send_wea <= "1";
						byteCounter := 0;
						behaviorGraphSendState <= writeData;
					
					when writeData =>
						behaviorgraph_send_addr <= incr_f(behaviorgraph_send_addr);
						behaviorgraph_send_data <= bg_output_data_int(byteCounter*8+7 downto byteCounter*8);
						behaviorgraph_send_wea <= "1";
						if (byteCounter < 3) then
						   byteCounter := byteCounter + 1;
							behaviorGraphSendState <= writeData;
						else
							behaviorgraph_startSending <= '1';
							behaviorGraphSendState <= idle;
						end if;
					
				end case;
			end if;
		end if;
	 end process;

end Behavioral;
