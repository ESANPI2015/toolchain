library ieee;
use ieee.std_logic_1164.ALL;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;

library work;
use work.bg_vhdl_types.all;
use work.bg_graph_@name@_config.all;
use work.bg_graph_@name@_comm_config.all;

entity bg_graph_@name@_comm is
    port(
    -- Data input from external interface(s)
        in_portId : in std_logic_vector(7 downto 0);
        in_data : in std_logic_vector(DATA_WIDTH-1 downto 0);
        in_req : in std_logic;
        in_ack : out std_logic;
    -- Data output from external interface(s)
        out_recvId : out std_logic_vector(7 downto 0);
        out_portId : out std_logic_vector(7 downto 0);
        out_data : out std_logic_vector(DATA_WIDTH-1 downto 0);
        out_req : out std_logic;
        out_ack : in std_logic;
    -- Internal Inputs
        in_sources : in DATA_PORT(NO_INTERNAL_INPUTS-1 downto 0);
    -- Internal Outputs
        out_sinks : out DATA_PORT(NO_INTERNAL_OUTPUTS-1 downto 0);
    -- Other signals
        halt : in std_logic;
        rst : in std_logic;
        clk : in std_logic
        );
end bg_graph_@name@_comm;

architecture Behavioral of bg_graph_@name@_comm is

    type ExternalInputStates is (idle, check, waiting, sync);
    signal ExternalInputState : ExternalInputStates;
    signal currInput : integer range 0 to NO_EXTERNAL_INPUTS-1;
    signal gotRequest : std_logic_vector(0 to NO_EXTERNAL_INPUTS-1);
    signal gotAllRequests : std_logic;

    type ExternalOutputStates is (idle, waiting, transmit);
    signal ExternalOutputState : ExternalOutputStates;
    signal currOutput : integer range 0 to NO_EXTERNAL_OUTPUTS-1;

    -- signals here
    signal src_to_graph : DATA_PORT(NO_INPUTS-1 downto 0);
    signal src_to_graph_req : DATA_SIGNAL(NO_INPUTS-1 downto 0);
    signal src_to_graph_ack : DATA_SIGNAL(NO_INPUTS-1 downto 0);

    signal graph_to_sink : DATA_PORT(NO_OUTPUTS-1 downto 0);
    signal graph_to_sink_req : DATA_SIGNAL(NO_OUTPUTS-1 downto 0);
    signal graph_to_sink_ack : DATA_SIGNAL(NO_OUTPUTS-1 downto 0);

begin
    -- Generate a source for each internal input
    -- TODO: Halt the sources if the external inputs have not yet arrived!
    GEN_SOURCES : for i in NO_INTERNAL_INPUTS-1 downto 0 generate
        source : entity work.bg_source(Behavioral)
        port map (
                    clk => clk,
                    rst => rst,
                    halt => halt,
                    in_port => in_sources(i),
                    out_port => src_to_graph(INTERNAL_INPUT_TO_GRAPH_INPUT(i)),
                    out_req  => src_to_graph_req(INTERNAL_INPUT_TO_GRAPH_INPUT(i)),
                    out_ack  => src_to_graph_ack(INTERNAL_INPUT_TO_GRAPH_INPUT(i))
                 );
    end generate;

    -- Instantiate the graph
    graph : entity work.bg_graph_@name@(Behavioral)
    port map (
                clk => clk,
                rst => rst,
                halt => halt,
                in_port => src_to_graph,
                in_req => src_to_graph_req,
                in_ack => src_to_graph_ack,
                out_port => graph_to_sink,
                out_req => graph_to_sink_req,
                out_ack => graph_to_sink_ack
             );

    -- Generate a sink for each internal output
    GEN_SINKS : for i in NO_INTERNAL_OUTPUTS-1 downto 0 generate
        sink : entity work.bg_sink(Behavioral)
        port map (
                    clk => clk,
                    rst => rst,
                    halt => halt,
                    out_port => out_sinks(i),
                    in_port => graph_to_sink(INTERNAL_OUTPUT_TO_GRAPH_OUTPUT(i)),
                    in_req =>  graph_to_sink_req(INTERNAL_OUTPUT_TO_GRAPH_OUTPUT(i)),
                    in_ack =>  graph_to_sink_ack(INTERNAL_OUTPUT_TO_GRAPH_OUTPUT(i))
                 );
    end generate;

    -- External input handling
    -- Wait for incoming data and pass it to the corresponding graph input
    -- Keep track of the number of updates / external input and trigger the graph iff
    -- a) all inputs have seen at least one update
    -- b) one input has seen a double update
    -- => UPSAMPLING
    -- Alternative: <- THIS IS IMPLEMENTED
    -- Do not pass data to graph but wait until all operands are available
    -- If a double update happens, the newer value overwrites the old one
    -- => DOWNSAMPLING
    gotAllRequests <= and_reduce(gotRequest);

    GEN_EXTERNAL_INPUT_ACK : if NO_EXTERNAL_INPUTS < 1 generate
        in_ack <= '1';
    end generate;

    GEN_EXTERNAL_INPUT_PROCESS : if NO_EXTERNAL_INPUTS > 0 generate
        ExternalInputProcess : process (clk)
        begin
            if clk'event and clk = '1' then
                if rst = '1' then
                    for i in NO_EXTERNAL_INPUTS-1 downto 0 loop
                        if (EXTERNAL_INPUT_HAS_BACKEDGE(i)) then
                            gotRequest(i) <= '1';
                        else
                            gotRequest(i) <= '0';
                        end if;
                        src_to_graph_req(EXTERNAL_INPUT_TO_GRAPH_INPUT(i)) <= '0';
                        src_to_graph(EXTERNAL_INPUT_TO_GRAPH_INPUT(i)) <= (others => '0');
                    end loop;
                    currInput <= 0;
                    in_ack <= '0';
                    ExternalInputState <= idle;
                else
                    in_ack <= '0';
                    ExternalInputState <= ExternalInputState;
                    case ExternalInputState is
                        when idle =>
                            -- Check if we got a request and the portId matches a external input id
                            if ((EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput) = to_integer(unsigned(in_portId))) and (in_req = '1')) then
                                -- We got a request, so store the data, ack it and signal the graph
                                -- Go and check if all requests have been gathered
                                -- NOTE: Multiple updates will override the old data!
                                in_ack <= '1';
                                gotRequest(currInput) <= '1';
                                src_to_graph(EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput)) <= in_data;
                                src_to_graph_req(EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput)) <= '1';
                                ExternalInputState <= check;
                            else
                                currInput <= currInput + 1;
                            end if;
                        when check =>
                            -- Check if we got all needed requests
                            if (gotAllRequests = '1') then
                                -- Reset all flags and currInput
                                gotRequest <= (others => '0');
                                currInput <= 0;
                                ExternalInputState <= waiting;
                            else
                                -- If not, go to idle and advance
                                currInput <= currInput + 1;
                                ExternalInputState <= idle;
                            end if;
                        when waiting =>
                            -- Wait until currInput has seen the data
                            src_to_graph_req(EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput)) <= '1';
                            if (src_to_graph_ack(EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput)) = '1') then
                                src_to_graph_req(EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput)) <= '0';
                                ExternalInputState <= sync;
                            end if;
                        when sync =>
                            -- Wait until the currInput is ready and advance to the next input
                            -- If the last input has been checked, reset and go to idle
                            src_to_graph_req(EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput)) <= '0';
                            if (src_to_graph_ack(EXTERNAL_INPUT_TO_GRAPH_INPUT(currInput)) = '0') then
                                ExternalInputState <= waiting;
                                currInput <= currInput + 1;
                                if (currInput = NO_EXTERNAL_INPUTS-1) then
                                    currInput <= 0;
                                    ExternalInputState <= idle;
                                end if;
                            end if;
                    end case;
                end if;
            end if;
        end process ExternalInputProcess;
    end generate;

    -- External output handling
    -- For every external output we have to check the corresponding output of the graph and produce a transmission request if needed
    GEN_EXTERNAL_OUTPUT_REQ : if NO_EXTERNAL_OUTPUTS < 1 generate
        out_req <= '0';
    end generate;

    GEN_EXTERNAL_OUTPUT_PROCESS : if NO_EXTERNAL_OUTPUTS > 0 generate
        ExternalOutputProcess : process (clk)
        begin
            if clk'event and clk = '1' then
                if rst = '1' then
                    for i in NO_EXTERNAL_OUTPUTS-1 downto 0 loop
                        graph_to_sink_ack(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(i)) <= '0';
                    end loop;
                    out_portId <= (others => '0');
                    out_recvId <= (others => '0');
                    out_data <= (others => '0');
                    out_req <= '0';
                    currOutput <= 0;
                    ExternalOutputState <= idle;
                else
                    out_req <= '0';
                    graph_to_sink_ack(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(currOutput)) <= '0';
                    ExternalOutputState <= ExternalOutputState;
                    case ExternalOutputState is
                        when idle =>
                            -- Check every output of the graph:
                            -- Iff there is new data, set parameters and register value
                            -- Iff not, try next output
                            if (graph_to_sink_req(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(currOutput)) = '1') then
                                out_recvId <= std_logic_vector(to_unsigned(EXTERNAL_OUTPUT_TO_RECVID(currOutput),8));
                                out_portId <= std_logic_vector(to_unsigned(EXTERNAL_OUTPUT_TO_SINKID(currOutput), 8));
                                out_data <= graph_to_sink(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(currOutput));
                                graph_to_sink_ack(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(currOutput)) <= '1';
                                ExternalOutputState <= waiting;
                            else
                                currOutput <= currOutput + 1;
                            end if;
                        when waiting =>
                            -- Wait until the output has seen our ack
                            graph_to_sink_ack(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(currOutput)) <= '1';
                            if (graph_to_sink_req(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(currOutput)) = '0') then
                                graph_to_sink_ack(EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT(currOutput)) <= '0';
                                out_req <= '1';
                                ExternalOutputState <= transmit;
                            end if;
                        when transmit =>
                            -- Wait until the packet starts transmitting
                            -- When we are finished, we advance to the next output to be checked (round robin arbitration)
                            out_req <= '1';
                            if (out_ack = '1') then
                                out_req <= '0';
                                currOutput <= currOutput + 1;
                                ExternalOutputState <= idle;
                            end if;
                    end case;
                end if;
            end if;
        end process ExternalOutputProcess;
    end generate;
end Behavioral;
