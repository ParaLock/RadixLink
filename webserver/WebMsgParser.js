class MsgParser {

    constructor() {

        this.m_lists = [];
        this.m_scalers = {};
    
        this.msg = "";
    }

    parse(incomingMsg) {   

        if(incomingMsg.length > 1) {

            var tokens = incomingMsg.split(';');
            
            for(var i = 0; i < tokens.length; i++) {

                if(tokens[i].length == 1) {
                    continue;
                }

                var keyVals = tokens[i].split('=');
                var listVals = keyVals[1].split('-');
                
                if(listVals.length > 1) {

                    var list = [];

                    for(var j = 0; j < listVals.length; j++) {

                        list.push(listVals[j]);
                    }

                    this.m_lists[keyVals[0]] = list;

                } else {

                    this.m_scalers[keyVals[0]] = keyVals[1];
                }

            }
        }
    }

    encode(name, val) {

        this.msg += name + "=" + val + ";";
    }

    encodeList(name, values) {

        this.msg += name + "=";

        for(var i = 0; i < values.length; i++) {

            this.msg += values[i];

            if(i != values.length - 1) {

                this.msg += "-";
            }
        }

        this.msg += ";";
    }

    reset() {
        
        this.msg = "";

        this.m_lists = {};
        this.m_scalers = {};
    }

    getMessage() {

        var temp = this.msg;
        temp = temp.substring(0, temp.length - 1);

        console.log("SENT MESSAGE" + temp);

        return temp;
    }

    get(name) {

        if(this.m_scalers.hasOwnProperty(name)) {

            var l = [];
            l.push(this.m_scalers[name]);

            return l;

        } else if(this.m_lists.hasOwnProperty(name)) {

            return this.m_lists[name];
        }
    }

    getScaler(l, def) {

        for(var i = 0; i < l.length; i++) {

            return l[i];
        }

        return def;
    }

};