library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

library work;
use work.bg_vhdl_types.all;
use work.bg_graph_@name@_config.all;

package bg_graph_@name@_comm_config is
    constant NO_INTERNAL_INPUTS  : integer := <numInternalInputs>;
    constant NO_EXTERNAL_INPUTS  : integer := <numExternalInputs>;
    constant NO_INTERNAL_OUTPUTS  : integer := <numInternalOutputs>;
    constant NO_EXTERNAL_OUTPUTS  : integer := <numExternalOutputs>;

    constant INTERNAL_INPUT_TO_GRAPH_INPUT : int_array_t(0 to NO_INTERNAL_INPUTS) :=
    (
        <internalInputSinkIdx0>
    );
    constant EXTERNAL_INPUT_TO_GRAPH_INPUT : int_array_t(0 to NO_EXTERNAL_INPUTS) :=
    (
        <externalInputSinkIdx0>
    );
    constant INTERNAL_OUTPUT_TO_GRAPH_OUTPUT : int_array_t(0 to NO_INTERNAL_OUTPUTS) :=
    (
        <internalOutputSrcIdx0>
    );
    constant EXTERNAL_OUTPUT_TO_GRAPH_OUTPUT : int_array_t(0 to NO_EXTERNAL_OUTPUTS) :=
    (
        <externalOutputSrcIdx0>
    );
    constant EXTERNAL_OUTPUT_TO_RECVID : int_array_t(0 to NO_EXTERNAL_OUTPUTS) :=
    (
        <externalOutputSinkId0>
    );
    constant EXTERNAL_OUTPUT_TO_SINKID : int_array_t(0 to NO_EXTERNAL_OUTPUTS) :=
    (
        <externalOutputSinkIdx0>
    );

	component bg_graph_@name@ is
        port(
            -- Inputs
                in_port : in DATA_PORT(NO_INPUTS-1 downto 0);
                in_req : in DATA_SIGNAL(NO_INPUTS-1 downto 0);
                in_ack : out DATA_SIGNAL(NO_INPUTS-1 downto 0);
            -- Outputs
                out_port : out DATA_PORT(NO_OUTPUTS-1 downto 0);
                out_req : out DATA_SIGNAL(NO_OUTPUTS-1 downto 0);
                out_ack : in DATA_SIGNAL(NO_OUTPUTS-1 downto 0);
            -- Other signals
                halt : in std_logic;
                rst : in std_logic;
                clk : in std_logic
            );
    end component;
end;
