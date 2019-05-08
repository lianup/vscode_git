var vm1 = new Vue({
    el:'#vm1',
    data:{
        message:'hello'
    }
});

var vm2 = new Vue({
    el:'#vm2',
    data:{
        rawHtml:'<h1>test</h1>'
    }
});

var href = {
    url:'http://www.baidu.com'
}




var vm3 = new Vue({
    el:'#vm3',
    data:{
        atrributeName:href
    }
});