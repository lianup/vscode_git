var app = new Vue({
    el: '#app',
    data:{
        message:'hello vue!'
    }
});      

var app2 = new Vue({

    el: '#app-2',
    data:{
        message:'页面加载于' + new Date().toLocaleDateString()
    }
});

var app3 = new Vue({
    el:'#app-3',
    data:{
        seen:true
    }
});

var app4 = new Vue({

    el:"#app-4",
    data:{
        todos:[
            {text:'learn js'},
            {text:'learn css'},
            {text:'learn html'}
        ]
    }
});

var app5 = new Vue({
    el:"#app-5",
    data:{
        message:'hello vue.js!'
    },
    methods:{
        reverseMessage: function() {
            this.message = this.message.split('').reverse().join('')
        }
    }
});

var app6 = new Vue({
    el:"#app-6",
    data:{
        message:"hello vue!"
    }
});

Vue.component('todo-item', {
    // todo-item 组件现在接受一个
    // "prop"，类似于一个自定义特性。
    // 这个 prop 名为 todo。
    props: ['todo'],
    template: '<li>{{ todo.text }}</li>'
  })

  var app7 = new Vue({
    el: '#app-7',
    data: {
      groceryList: [
        { id: 0, text: '蔬菜' },
        { id: 1, text: '奶酪' },
        { id: 2, text: '随便其它什么人吃的东西' }
      ]
    }
  })

  var obj = {
      foo: 'bar'
  }
  // 此属性会导致obj不再变化
    Object.freeze(obj);
  var app8 = new Vue({
    el: '#app-8',
    data: obj
  });
  
  var data = { a: 1 }
var vm = new Vue({
  el: '#example',
  data: data
})
//Vue 实例还暴露了一些有用的实例属性与方法。它们都有前缀 $，以便与用户定义的属性区分开来
vm.$data === data // => true
vm.$el === document.getElementById('example') // => true
vm.$watch('a', function (newValue, oldValue) {
    // 这个回调将在 `vm.a` 改变后调用
  })

  new Vue({
    data: {
      a: 1
    },
    // 声明周期钩子函数,在一个示例被创建后执行
    created: function () {
      // `this` 指向 vm 实例
      console.log('a is: ' + this.a)
    }
  })