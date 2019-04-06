class MsgParser {

    constructor() {

        this.m_lists = [];
        this.m_scalers = {};
    
        this.msg = "";
    }

    parse(incomingMsg) {   

        var tokens = incomingMsg.split(';');
        
        for(var i = 0; i < tokens.length; i++) {

            var keyVals = tokens[i].split('=');
            var listVals = keyVals[1].split('-');
            
            if(listVals.length > 1) {

                var list = [];

                for(var j = 0; j < listVals.length; j++) {

                    list.push(listVals[i]);
                }

                this.m_lists[keyVals[0]] = list;

            } else {

                this.m_scalers[keyVals[0]] = keyVals[1];
            }

        }

    }

    encode(name, val) {

        this.msg += name + "=" + val + ";";
    }

    encode(name, values) {

        this.msg += name + "=";

        for(var i = 0; i < values.length; i++) {

            this.msg += values[i];

            if(i != values.length - 1) {

                this.msg += "-";
            }
        }
    }

    reset() {
        
        this.msg = "";

        this.m_lists = {};
        this.m_scalers = {};
    }

    getMessage() {

        return this.msg;
    }

    getScaler(name) {

        return m_scalers[name];
    } 

   getList(name) {

        return m_lists[name];
    }

};